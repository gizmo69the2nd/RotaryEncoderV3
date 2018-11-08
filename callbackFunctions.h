#ifndef CALLBACK_H
#define CALLBACK_H

#include <Arduino.h>
#include "types.h"

extern uint8_t state;

void callbackInput();
void callbackReset();
void callbackClear();
void callbackRead();


#endif