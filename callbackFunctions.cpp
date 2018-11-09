#include "callbackFunctions.h"

void callbackInput()
{
  //Serial.println("Input timer callback");
  state = CHECK_POS_DIR;
}

void callbackReset()
{
  //Serial.println("reset timer");
  state = RESET;
}

void callbackClear()
{
  //Serial.println("clear timer");
  state = NOK;
}

void callbackRead()
{
  state = READ_ENC;
}

void callbackBlink()
{
  //Serial.println("Blink");
  digitalWrite(RED_LED,digitalRead(GREEN_LED));
  digitalWrite(GREEN_LED,!digitalRead(GREEN_LED));

}