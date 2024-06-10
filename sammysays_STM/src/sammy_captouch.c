// includes
#include <stdlib.h>
#include <stdio.h>
#include <Board.h>
#include <timers.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <stm32f4xx_hal_tim.h>
#include "sammy_captouch.h"

// number of period samples to take from each capacitive touch sensor
#define SAMPLESIZE 300
// period over which the captouch sensors are considered touched, in us
#define CUTOFF 300

// global variables for timing purposes
int prev1, current1, period1, num1 = 0;
int prev2, current2, period2, num2 = 0;
int prev3, current3, period3, num3 = 0;
int prev4, current4, period4, num4 = 0;

// arrays to hold period samples for each captouch
int periodReadings1[SAMPLESIZE];
int periodReadings2[SAMPLESIZE];
int periodReadings3[SAMPLESIZE];
int periodReadings4[SAMPLESIZE];

void SammyCaptouch_Init(void) {
    //Configure GPIO pin PB5 PB4
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    // following line configures them to look for rising edges
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // now try PA7 PA6
    GPIO_InitTypeDef GPIO_InitStruct2 = {0};
    GPIO_InitStruct2.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct2.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct2.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct2);
 
    // EXTI interrupts init
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  
    // the rest of the function goes here
    TIMER_Init();
}
// external interrupt ISR for rising edge of pin PB4
void EXTI4_IRQHandler(void){
    // EXTI Line interrupt detected
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET){
        // clear interrupt flag
        __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_4);

        // update current/prev times in us
        prev1 = current1;
        current1 = TIMERS_GetMicroSeconds();
        // subtract to get the period
        period1 = current1 - prev1;
        
        // fill this captouch's period readings array
        periodReadings1[num1] = period1;
        // update array index/roll over if there are already 300 readings
        num1++;
        if (num1 >= SAMPLESIZE){
            num1 = 0;
        }

    }
}
// external interrupt ISR for rising edge of pin PB5  
void EXTI9_5_IRQHandler(void) {
    // EXTI line interrupt detected 
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5); // clear interrupt flag
  
        // anything that needs to happen on rising edge of PB5 (ENC_B)
        prev2 = current2;
        current2 = TIMERS_GetMicroSeconds();
        period2 = current2 - prev2;
        periodReadings2[num2] = period2;
        num2++;
        if (num2 >= SAMPLESIZE){
            num2 = 0;
        }
        //printf("Hello\n");
        //printf("%u\n", current_time);
        }

    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_7) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_7);

        // same procedure as one and two
        prev3 = current3;
        current3 = TIMERS_GetMicroSeconds();
        period3 = current3 - prev3;
        periodReadings3[num3] = period3;
        num3++;
        if (num3 >= SAMPLESIZE){
            num3 = 0;
        }
    }

    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6)){
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);

        // same procedure again
        //printf("hello??\n");
        prev4 = current4;
        current4 = TIMERS_GetMicroSeconds();
        period4 = current4-prev4;
        periodReadings4[num4] = period4;
        num4++;
        if (num4 >= SAMPLESIZE){
            num4 = 0;
        }
    }
}




// Moving average helper function: calculates the average of a sample of size SAMPLESIZE
int movingAverage(int samples[SAMPLESIZE]){
    int avg = 0;
    for (int i = 0; i < SAMPLESIZE; i++){
        avg = avg + samples[i];
    }
    avg = avg/SAMPLESIZE;
    return avg;
}


// CAPTOUCH STATUS FUNCTIONS
// If the moving average of the period is greater than a predefined cutoff value-- here
// pound defined at the head of the file-- returns TRUE, otherwise FALSE
// CUTOFF = 300 worked well in Lab 2, adjust as necessary
char SammyCaptouch_1isTouched(void){
    //printf("cap1 touched");
    //printf("%d\n", movingAverage(periodReadings1));
    if(movingAverage(periodReadings1) > 130){
        return TRUE;
    }
    return FALSE;
}

char SammyCaptouch_2isTouched(void){
    //printf("cap2 touched");
    //printf("%d\n", movingAverage(periodReadings2));
    if(movingAverage(periodReadings2) > 130){
        return TRUE;
    }
    return FALSE;
}

char SammyCaptouch_3isTouched(void){
    //printf("cap3 touched");
    //printf("%d\n", movingAverage(periodReadings3));
    if(movingAverage(periodReadings3) > 580){
        return TRUE;
    }
    return FALSE;
}

char SammyCaptouch_4isTouched(void){
    //printf("cap4 touched");
    //printf("%d\n", movingAverage(periodReadings4));
    if(movingAverage(periodReadings4) > 140){
        return TRUE;
    }
    return FALSE;
}

