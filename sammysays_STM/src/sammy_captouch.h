/*******************************************************************************
 * INCLUDES                                                                    *
 ******************************************************************************/
#include <Board.h>
#include <timers.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <stm32f4xx_hal_tim.h>

// Initializes the four capacitive touch sensor reading module.
void SammyCaptouch_Init(void);

// Different functions for each capacitive touch sensor allow us to tell when they are being
// touched simultaneously.

// CAPTOUCH 1: PB4
// CAPTOUCH 2: PB5
// CAPTOUCH 3: PB7
// CAPTOUCH 4: PB14
// Please connect the circuits accordingly.
// Use these functions to access the status of each captouch sensor in the game's state machine.
// Returns TRUE when touched and FALSE when untouched.

char SammyCaptouch_1isTouched(void);

char SammyCaptouch_2isTouched(void);

char SammyCaptouch_3isTouched(void);

char SammyCaptouch_4isTouched(void);

