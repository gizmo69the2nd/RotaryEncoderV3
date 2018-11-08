#include "helper_functions.h"

void setLED(Led_Color c)
{
  switch(c)
  {
    case RED:
      Serial.println("Setting LED RED");
      digitalWrite(RED_LED,HIGH);
      digitalWrite(GREEN_LED,LOW);
      break;

    case GREEN:
      Serial.println("Setting LED GREEN");
      digitalWrite(RED_LED,LOW);
      digitalWrite(GREEN_LED,HIGH);
      break;

    case OFF:
      Serial.println("Turning LED off");
      digitalWrite(RED_LED,LOW);
      digitalWrite(GREEN_LED,LOW);
      break;
  }
}

InputValue readInputs(InputValue prev,uint8_t newVal)
{
  //Read the input value from the serial register and determine the direction
  
  //uint8_t newVal = read_shift();
  uint8_t oldVal = prev.position;
  Direction dir = STOP;

  if ((oldVal == 1)&&(newVal == 24))
  {
    dir = CCW;
  }
  else if ((oldVal == 24)&&(newVal == 1))
  {
    dir = CW;
  }
  else if (newVal > oldVal)
  {
    dir = CW;
  }
  else if (newVal < oldVal)
  {
    dir = CCW;
  }
  else if (newVal == oldVal)
  {
    dir = prev.direction;
  }
  
  return {newVal,dir};
}

boolean checkInput(InputValue v,uint8_t index, InputValue *pSecretKey)
{
  Serial.println("Checking input: " + String(v.position) + " " + dirToString(v.direction));
  //Serial.println("Index: " + String(index));
  //Serial.print("pointer thing: ");

  return ((v.position == (pSecretKey+index)->position) && (v.direction == (pSecretKey+index)->direction));
}

String dirToString(uint8_t d)
{
  switch (d)
  {
    case 0:
      return "STOP";
    case 1:
      return "CW";
    case 2:
      return "CCW";
    default:
      return "STOP";
  }
}
