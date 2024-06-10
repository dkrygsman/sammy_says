#include <stdlib.h>
#include <stdio.h>
#include "Board.h"
#include "leds.h"
#include "buttons.h"
#include "sammy_captouch.h"
#include "math.h"
#include "Oled.h"
#include "OledDriver.h"
#include "pwm.h"


enum STATES {CHILL, PROMPT, DELAY, READING, CORRECT, INCORRECT};
enum STATES state;
int timeprev, timecurrent, button, n, level, highscore = 0;
char correct, incorrect = FALSE;
int array[100];
char str[50];

// Function to initialize the state machine in state 'CHILL'
void statemachine_init(void){
    state = CHILL; // state to CHILL
    level = 1; // level to 1
    n = 0; // reset position in the sequence
    timeprev = 0;
    timecurrent = 0;
}

// Helper function that makes it easier to check the captouch library results against the prompt
uint8_t captouch(void){
    uint8_t num = 0;
    if (SammyCaptouch_1isTouched()){
        return 1;
    } else if (SammyCaptouch_2isTouched()){
        return 2;
    } else if (SammyCaptouch_3isTouched()){
        return 3;
    } else if (SammyCaptouch_4isTouched()){
        return 4;
    } else {
        return 0;
    }
}

// Helper function to change the raw values from buttons_state() to more usable ints
// using bitmasking
int getbutton(void){
    if (buttons_state() >= 15){
        return 0;
    } else if ((~buttons_state() & 0b00001111) == 1){
        return 1;
    } else if ((~buttons_state() & 0b00001111) == 2){
        return 2;
    } else if ((~buttons_state() & 0b00001111) == 4){
        return 3;
    } else if ((~buttons_state() & 0b00001111) == 8){
        return 4;
    } else {
        return 5;
    }
}

// Test version of the state machine that only uses buttons, not captouches
void statemachine(void){
    if (state == CHILL){ // first state: CHILL, where it waits for a game to start
        // set_leds(1);
        if (buttons_state() < 15){ // if any button is pressed:
            int i;
            for (i = 0; i < 100; i++){ // generate full 100-value sequence
                array[i] = rand()%4 + 1; // of random numbers from 1 to 4
                printf("%d ", array[i]);
            }
            printf("\n");
            timeprev = TIMERS_GetMilliSeconds(); // reset prev time

            state = PROMPT; // move to state PROMPT
        }
    } else if (state == PROMPT){

        timecurrent = TIMERS_GetMilliSeconds(); // get time
        // printf("%d\n", timecurrent);
        if (timecurrent - timeprev >= 500 && n < level){ // go through the sequence up to the current
            timeprev = TIMERS_GetMilliSeconds();            // level and then display on PWM/leds
            printf("Target: %d\n", array[n]);
            set_leds(pow(2, (array[n]-1))); // Set LEDS according to the current n
            PWM_SetDutyCycle(PWM_0, 20);
            PWM_SetFrequency(500*array[n]);
            n++;
            state = DELAY; // same state
        } else if (timecurrent - timeprev >= 500){ // once the sequence has finished
            set_leds(0); // turn off the leds
            PWM_Stop(PWM_0);
            n = 0; // reset n to zero for reading purposes
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            state = READING; // go to the reading state
        }
    } else if (state == DELAY){
        timecurrent = TIMERS_GetMilliSeconds();
        if (timecurrent - timeprev >= 1000){
            PWM_Stop(PWM_0);
            set_leds(0);
            state = PROMPT;
        }
    } else if (state == READING){ // in this state, the state machine looks for the correct touch
        
        //set_leds(4);
        timecurrent = TIMERS_GetMilliSeconds(); 
        
        if (getbutton() == array[n]){ // if the current n == the button press
            printf("Pressed: %d\n", getbutton());
            printf("CORRECT\n");
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            n++; // increment n and go to correct state
            state = CORRECT;
        } else if ((getbutton() != 0 && getbutton()!=array[n]) || timecurrent-timeprev >= 2000){
            // if the wrong button is pressed OR the game times out
            printf("Pressed: %d | True %d | Index %d\n", getbutton(), array[n], n);
            printf("INCORRECT\n");
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            state = INCORRECT; // go to the incorrect state
        }
    } else if (state == CORRECT){ // state that shows the leds when you get it right
        set_leds(15);
        PWM_SetDutyCycle(PWM_0, 20);
        PWM_SetFrequency(500*array[n-1]);
        timecurrent = TIMERS_GetMilliSeconds();
        if (timecurrent-timeprev >= 500 && getbutton() == 0 && n < level){ // only go here when the button is no longer pressed
            timeprev = TIMERS_GetMilliSeconds();
            PWM_Stop(PWM_0);                            // (otherwise it goes too fast and is annoying/confusing)
            set_leds(0);
            state = READING; // assuming you haven't beat the level yet, go back to READING
        } else if (timecurrent-timeprev >= 500 && getbutton() == 0){ // if you HAVE beat the level
            set_leds(0);
            PWM_Stop(PWM_0);
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            level++; // increment level
            if (level > highscore){ // check for new highscore
                highscore = level;
            }
            n = 0; // reset n
            printf("LEVEL %d\n", level);
            state = PROMPT; // go to prompt to continue
        }
    } else if (state == INCORRECT){ // incorrect state for getting it wrong
        set_leds(255);
        timecurrent = TIMERS_GetMilliSeconds();
        if (timecurrent - timeprev >= 1500){
            
            
            level = 1; // reset level back to 1
            n = 0; // reset n
            state = CHILL; // go back to the chill state
        }
    }
}

// Actual state machine with capacitive touch sensors
void statemachine_CAPTOUCHES(void){
    if (state == CHILL){ // first state: CHILL, where it waits for a game to start
        if (captouch() != 0){ // if any button is pressed:
            int i;
            for (i = 0; i < 100; i++){ // generate full 100-value sequence
                array[i] = rand()%4 + 1; // of random numbers from 1 to 4
                printf("%d ", array[i]); // print them through serial terminal for testing
            }
            printf("\n");
            timeprev = TIMERS_GetMilliSeconds(); // reset prev time

            state = PROMPT; // move to state PROMPT
        }
    } else if (state == PROMPT){

        timecurrent = TIMERS_GetMilliSeconds(); // get time
        // printf("%d\n", timecurrent);
        if (timecurrent - timeprev >= 500 && n < level){ // go through the sequence up to the current
            timeprev = TIMERS_GetMilliSeconds();            // level and then display on PWM/leds
            printf("Target: %d\n", array[n]); // makes testing easier
            if (array[n] == 1){ // this bit turns on the leds in order to show the prompt
                PWM_SetDutyCycle(PWM_1, 100);
            } else if (array[n] == 2){
                PWM_SetDutyCycle(PWM_2, 100);
            } else if (array[n] == 3){
                PWM_SetDutyCycle(PWM_3, 100);
            } else {
                PWM_SetDutyCycle(PWM_4, 100);
            }
            set_leds(pow(2, (array[n]-1))); // Set STM LEDS according to the current n
            PWM_SetDutyCycle(PWM_0, 20); // this makes the speaker blare a tone according to the prompt
            PWM_SetFrequency(300*array[n]);
            n++;
            state = DELAY; // go to DELAY
        } else if (timecurrent - timeprev >= 500){ // once the sequence has finished
            set_leds(0); // turn off the STM leds
            PWM_SetDutyCycle(PWM_0, 0); // turn off the tone
            PWM_SetDutyCycle(PWM_1, 0); // turn off the big leds
            PWM_SetDutyCycle(PWM_2, 0);
            PWM_SetDutyCycle(PWM_3, 0);
            PWM_SetDutyCycle(PWM_4, 0);

            n = 0; // reset n to zero for reading purposes
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            state = READING; // go to the reading state
        }
    } else if (state == DELAY){ // this state implements a brief delay between tones for prompt-reading clarity
        timecurrent = TIMERS_GetMilliSeconds();
        if (timecurrent - timeprev >= 1000){
            PWM_SetDutyCycle(PWM_0, 0); // turn everything off so that the tones are separated
            PWM_SetDutyCycle(PWM_1, 0);
            PWM_SetDutyCycle(PWM_2, 0);
            PWM_SetDutyCycle(PWM_3, 0);
            PWM_SetDutyCycle(PWM_4, 0);
            set_leds(0);
            state = PROMPT;
        }
    } else if (state == READING){ // in this state, the state machine looks for the correct touch
        

        timecurrent = TIMERS_GetMilliSeconds(); 
        
        if (captouch() == array[n]){ // if the current n == the button press
            printf("Pressed: %d\n", captouch());
            printf("CORRECT\n");
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            n++; // increment n and go to correct state
            state = CORRECT;
        } else if ((captouch() != 0 && captouch() !=array[n]) || timecurrent-timeprev >= 2000){
            // if the wrong button is pressed OR the game times out
            printf("Pressed: %d | True %d | Index %d\n", captouch(), array[n], n);
            printf("INCORRECT\n");
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            state = INCORRECT; // go to the incorrect state
        }
    } else if (state == CORRECT){ // state that shows the leds when you get it right
        set_leds(15); // light up half the STM leds to distinguish from incorrect state
        //PWM_Start(PWM_0);
        PWM_SetDutyCycle(PWM_0, 20);
        PWM_SetFrequency(300*array[n-1]); // tone associated with the button you pressed

        if (array[n - 1] == 1){ // led associated with the button you pressed
                PWM_SetDutyCycle(PWM_1, 100);
            } else if (array[n - 1] == 2){
                PWM_SetDutyCycle(PWM_2, 100);
            } else if (array[n - 1] == 3){
                PWM_SetDutyCycle(PWM_3, 100);
            } else {
                PWM_SetDutyCycle(PWM_4, 100);
        }

        timecurrent = TIMERS_GetMilliSeconds();
        if (timecurrent-timeprev >= 500 && captouch() == 0 && n < level){ // only go here when the button is no longer pressed
            timeprev = TIMERS_GetMilliSeconds();
            PWM_SetDutyCycle(PWM_0, 0);  // turn everything off  
            PWM_SetDutyCycle(PWM_1, 0);
            PWM_SetDutyCycle(PWM_2, 0);
            PWM_SetDutyCycle(PWM_3, 0);
            PWM_SetDutyCycle(PWM_4, 0);                         
            set_leds(0);
            state = READING; // assuming you haven't beat the level yet, go back to READING
        } else if (timecurrent-timeprev >= 500 && captouch() == 0){ // if you HAVE beat the level
            set_leds(0);
            PWM_SetDutyCycle(PWM_0, 0); // turn everything off
            PWM_SetDutyCycle(PWM_1, 0);
            PWM_SetDutyCycle(PWM_2, 0);
            PWM_SetDutyCycle(PWM_3, 0);
            PWM_SetDutyCycle(PWM_4, 0);
            timeprev = TIMERS_GetMilliSeconds(); // reset time
            level++; // increment level
            if (level > highscore){ // check for new highscore
                highscore = level;
            }
            n = 0; // reset n
            printf("LEVEL %d\n", level);
            state = PROMPT; // go to prompt to continue
        }
    } else if (state == INCORRECT){ // incorrect state for when you get it wrong
        set_leds(255);
        timecurrent = TIMERS_GetMilliSeconds();
        if (timecurrent - timeprev >= 1500){
            
            
            level = 1; // reset level back to 1
            n = 0; // reset n
            state = CHILL; // go back to the chill state where you can start a new game
        }
    }
}

int main(void){

    // Initialize everything including PWM
    BOARD_Init();
    SammyCaptouch_Init();
    OledInit();
    TIMER_Init();
    BUTTONS_Init();
    LEDS_Init();
    PWM_Init();
    PWM_AddPin(PWM_0);
    PWM_SetDutyCycle(PWM_0, 10);
    PWM_SetFrequency(500);
    PWM_SetDutyCycle(PWM_0, 0);

    PWM_AddPin(PWM_1);
    PWM_SetDutyCycle(PWM_1, 0);


    

    statemachine_init();

    
    while(TRUE){
        // Continuously print level and highscore
        sprintf(str, "Level: %d\nHighscore: %d", level, highscore);
        OledDrawString(str);
        OledUpdate();
        //SammyCaptouch_4isTouched();
        statemachine_CAPTOUCHES();


    }
}
