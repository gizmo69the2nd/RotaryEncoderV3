#ifndef FSMFUNCTIONS_H
#define FSMFUNCTIONS_H

#include <Arduino.h>
#include <TaskScheduler.h>
#include "types.h"
#include "config.h"
//#include "callbackFunctions.h"
#include "helper_functions.h"

extern uint8_t index;
extern uint8_t keySize;

extern InputValue currentInput;
extern InputValue prevInput;

extern InputValue secretKey[MAX_KEY_SIZE];

uint8_t fsmReset();
uint8_t fsmReadEnc();
uint8_t fsmCheckPosDir();
uint8_t fsmCheckDirChange();
uint8_t fsmRestartTimer();
uint8_t fsmOK();
uint8_t fsmNOK();
uint8_t fsmSolved();


//Define task scheduler
extern Scheduler ts;

extern Task tInputTimer;
extern Task tResetTimer;
extern Task tClearTimer;

#endif