#ifndef HELPER_FUNCTION_H
#define HELPER_FUNCTION_H

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "shiftIn.h"
#include <EEPROM.h>




void setLED(Led_Color c);

InputValue readInputs(InputValue prev,uint8_t newVal);
boolean checkInput(InputValue v,uint8_t index, InputValue *pSecretKey);
String dirToString(uint8_t d);
void printSecretKey(InputValue *pSecretKey,uint8_t size);

void writeEEPROM(InputValue *pSecretKey, uint8_t size);
boolean readEEPROM(InputValue *pSecretKey, uint8_t *size);
void dumpEEPROM();
void clearEEPROM();

#endif
