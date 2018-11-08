#ifndef HELPER_FUNCTION_H
#define HELPER_FUNCTION_H

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "shiftIn.h"




void setLED(Led_Color c);

InputValue readInputs(InputValue prev,uint8_t newVal);
boolean checkInput(InputValue v,uint8_t index, InputValue *pSecretKey);
String dirToString(uint8_t d);

#endif
