#ifndef ROTARY_H
#define ROTARY_H

#include "config.h"
#include "types.h"
//#include "fsmFunctions.h"
//#include "callbackFunctions.h"
#include <TaskScheduler.h>
#include "helper_functions.h"

uint8_t state;        //current FSM state
uint8_t index;
uint8_t keySize;

InputValue currentInput;
InputValue prevInput;

InputValue secretKey[MAX_KEY_SIZE];


uint8_t fsmReset();
uint8_t fsmReadEnc();
uint8_t fsmCheckPosDir();
uint8_t fsmCheckDirChange();
uint8_t fsmRestartTimer();
uint8_t fsmOK();
uint8_t fsmNOK();
uint8_t fsmSolved();


void callbackInput();
void callbackReset();
void callbackClear();

//Define task scheduler
Scheduler ts;

Task tInputTimer(INPUT_TIME, TASK_FOREVER, &callbackInput, &ts, false);
Task tResetTimer(RESET_TIME, TASK_FOREVER, &callbackReset, &ts, false);
Task tClearTimer(CLEAR_TIME, TASK_FOREVER, &callbackClear, &ts, false);

#endif
