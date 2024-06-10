/******************************************************
 * File:   sammy_says.c
 * Author: Daniel Krygsman, Teresa Begley
 *
 * Created on Febuary 16, 2024
 *///**************************************************

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <AD.h>
#include <BOARD.h>
#include <Buttons.h>
#include <pwm.h>
#include <serial.h>
#include <timers.h>
#include <ToneGeneration.h>
#include <I2C.h>

static unsigned int state, buttons, high_score, level, n, play, score;
static unsigned int flag1, flag2, flag3;
static unsigned int array[100];
static unsigned int last_time, print_time, flash_last_time, flash_count;

#define Main

#ifdef Main
int main(void) 
{
    TIMERS_Init();
    BOARD_Init();
    
    flag1 = 0;
    flag2 = 0;
    flag3 = 0;
    play = 0;
    state = 1;
    level = 0;
    high_score = 0;
    n = 0;
    TRISE = 0x00;
    score = 0;
    print_time = 0;
    last_time = 0;
    flash_count = 0;
    
    while(1)
    {
        
        if(abs(print_time - TIMERS_GetMilliSeconds()) > 500){
            printf("n=");
            printf("%d", n);
            printf("\nlevel=");
            printf("%d", level);
            printf("\nhigh_score=");
            printf("%d", high_score);
            printf("\nstate=");
            printf("%d", state);                                //print statements for debugging 
            printf("\n");
            printf("%d", array[0]);
            printf("\n");
            printf("%d", array[1]);
            printf("\n");
            printf("%d", array[2]);
            printf("\n");
            printf("%d", array[3]);
            printf("\n");
            printf("%d", array[4]);
            printf("\n\n");
            print_time = TIMERS_GetMilliSeconds();
        }
        
      
        
        buttons = ButtonsCheckEvents();                 //raise button 1-3 flags for prompts
                                                        //also check play button 
        
        if(buttons == BUTTON_EVENT_1DOWN)
        {
            flag1 = 1;
        }
        if(buttons == BUTTON_EVENT_2DOWN)
        {
            flag2 = 1;
        }
        if(buttons == BUTTON_EVENT_3DOWN)
        {
            flag3 = 1;
        }
        if(buttons == BUTTON_EVENT_4DOWN)
        {
            play = 1;
        }
        
        
        // state 1: Chill
        // state 2: Loading 
        // state 3: Prompt 
        // state 4: Delay
        // state 5: Reading 
        // state 6: Correct
        // state 7: Incorrect
    
        switch(state)
        {
        case 1:                             //chill state waits for play button goes to loading state 
        {
            LATE = 0b10000000;
            if(play == 1)
            {
                state = 2;
                play = 0;
            }
        }
        break;
        case 2:                         //loading state 
        {
            LATE = 0b01000000;
            level = level + 1;          //set level value to level and n
            n = level;
                
                
            for(int i = 0; i < n; i ++)         //for loop for assigning array[0]-array[n] values
            {
                array[i] = rand() % 3 + 1;
                if(i == n - 1)
                {
                    n = 0;
                    last_time = TIMERS_GetMilliSeconds();       //set time for delay into prompt state
                    state = 3;
                }
            } 
                
        }
        break;
        case 3:                    //Prompt state 
        {
            
            if(abs(last_time - TIMERS_GetMilliSeconds()) > 250)     //delay for time between prompts 
            {
            
                if(array[n]==1)                     //if statements for lighting up prompts 
                {
                    LATE = 0b00100001;
                    last_time = TIMERS_GetMilliSeconds();
                    state = 4;
                }   
                if(array[n]==2)
                {
                    LATE = 0b00100010;
                    last_time = TIMERS_GetMilliSeconds();
                    state = 4;
                }
                if(array[n]==3)
                {
                    LATE = 0b00100100;
                    last_time = TIMERS_GetMilliSeconds();
                    state = 4;
                }
            }
            
        }
        break;
        case 4:
        {
            if(abs(last_time - TIMERS_GetMilliSeconds()) > 1500)        //time for length of prompt
            {
                LATE = 0b00000000;
                n = n + 1;                                  //inc n for next prompt
                state = 3;                                  //back to prompt
                last_time = TIMERS_GetMilliSeconds();
                if(n == level)                              //if no more values in array
                {
                    state = 5;                              //override state to no go to reading 
                    n = 0;                                  //n = 0 for reading first array value
                }
            }
            
            
        }
        break;
        case 5:                         //reading state 
        {                              
            
            LATE = 0b00001000;
              
            if((array[n] == 1)&&(flag1 == 1)){          //if button press is correct 
                state = 6;
                flag1 = 0;
                flag2 = 0;
                flag3 = 0;
            }
            if((array[n] == 2)&&(flag2 == 1)){          //should combine all correct readings bc they have actions
                state = 6;
                flag1 = 0;
                flag2 = 0;
                flag3 = 0;
                }
            if((array[n] == 3)&&(flag3 == 1)){
                state = 6;
                flag1 = 0;
                flag2 = 0;
                flag3 = 0;
            }
            if((array[n] == 1)&&((flag2 ==1)||(flag3 ==1))){        //incorrect readings
                state = 7;
                flag1 = 0;
                flag2 = 0;                                  //should combine these as well
                flag3 = 0;
                last_time = TIMERS_GetMilliSeconds();
                LATE = 0b00000000;
            }
            if((array[n] == 2)&&((flag1 ==1)||(flag3 ==1))){
                state = 7;
                flag1 = 0;
                flag2 = 0;
                flag3 = 0;
                last_time = TIMERS_GetMilliSeconds();
                LATE = 0b00000000;                              //this is for flashing xor boolean algebra
            }
            if((array[n] == 3)&&((flag1 ==1)||(flag2 ==1))){
                state = 7;
                flag1 = 0;
                flag2 = 0;
                flag3 = 0;
                last_time = TIMERS_GetMilliSeconds();
                LATE = 0b00000000;
            }
            if(abs(last_time - TIMERS_GetMilliSeconds()) > 4000){       //this is the timed out 4 sec
                state = 7;
                flag1 = 0;
                flag2 = 0;
                flag3 = 0;
                last_time = TIMERS_GetMilliSeconds();
                LATE = 0b00000000;
            }
        }
        break;
        case 6:                         //correct state 
        {
            LATE = 0b00000000;
            if(n == (level-1)){         //if this is last value in array inc score
                score = score +1;
                state = 2;              //go to loading instead of back to reading
            }
            if(n < (level-1)){          //not last array value
                n = n +1;               //inc n go back to reading 
                state = 5;
            }
                
        }
        break;
        case 7:                     //incorrect state 
        {
            
            
            if(abs(last_time - TIMERS_GetMilliSeconds()) > 100)     //timer for flashing 
            {
                LATE = LATE ^ 0b11111111;       //toggle
                
                last_time = TIMERS_GetMilliSeconds();
                flash_count = flash_count + 1;
                
                if(flash_count >= 20)           //should happen after 20 iterations 
                {
                    if(score > high_score)      //if score is new high set high_score
                    {
                        high_score = score;
                    }

                    score = 0;              //restart game 
                    level = 0;
                    state = 1;
                    flash_count = 0;
                }
                
            }
        }
        }
    }
}
#endif





