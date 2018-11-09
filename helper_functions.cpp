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

    case ORANGE:
      Serial.println("Setting LED ORANGE");
      digitalWrite(RED_LED,HIGH);
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

void printSecretKey(InputValue *pSecretKey,uint8_t size)
{
  Serial.println("Keysize: " + String(size));
  for (uint8_t i = 0; i<size; i++)
  {
    Serial.print("[" + String((pSecretKey+i)->position) + " - " + dirToString((pSecretKey+i)->direction) + "] ");
  }
  Serial.println("");
}

uint8_t calcLRC(InputValue *pSecret, uint8_t size)
{
  uint8_t LRC = 0;
  for (uint8_t i = 0; i< size; i++)
  {
    LRC ^= (pSecret+i)->position;
    //Serial.println((pSecret+i)->position);
    LRC ^= (pSecret+i)->direction;
    //Serial.println((pSecret+i)->direction);
  }
  return LRC;
}


void writeEEPROM(InputValue *pSecretKey, uint8_t size)
{
  Serial.println("Size: " + String(size));
  uint8_t indexEEPROM = 0;
  EEPROM.put(indexEEPROM++,size);

  for (uint8_t indexKey = 0; indexKey < size; indexKey++)
  {
    Serial.println("EEPROM: " + String(indexEEPROM) + " index: " + String(indexKey) + " pos:" +  (pSecretKey+indexKey)->position);
    EEPROM.write(indexEEPROM,(pSecretKey+indexKey)->position);
    indexEEPROM++;

    Serial.println("EEPROM: " + String(indexEEPROM) + " index: " + String(indexKey) + " dir:" +  (pSecretKey+indexKey)->direction);
    EEPROM.write(indexEEPROM,(pSecretKey+indexKey)->direction);
    indexEEPROM++;
  }
  uint8_t lrc = calcLRC(pSecretKey,size);
  Serial.println("LRC: " + String(lrc));
  EEPROM.write(indexEEPROM,lrc);
}

void clearEEPROM()
{
  Serial.println("Clearing EEPROM");
  for(uint16_t i=0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i,i);
  }
}


boolean readEEPROM(InputValue *pSecretKey, uint8_t *size)
{
  uint8_t lrc = 0;
  uint8_t indexEEPROM = 0;
  *size = EEPROM.read(indexEEPROM++);
  //Serial.println((uint8_t)*size);
  for (uint8_t indexKey = 0; indexKey < *size; indexKey++)
  {
    //Serial.println("EEPROM index: " + String(indexEEPROM) + " Key index: " + String(indexKey) + "pos: " + EEPROM.read(indexEEPROM));
    (pSecretKey+indexKey)->position = EEPROM.read(indexEEPROM);
    indexEEPROM++;

    //Serial.println("EEPROM index: " + String(indexEEPROM) + " Key index: " + String(indexKey) + "dir: " + EEPROM.read(indexEEPROM));
    (pSecretKey+indexKey)->direction = EEPROM.read(indexEEPROM);
    indexEEPROM++;
  }
  lrc = EEPROM.read(indexEEPROM);
  if (lrc == calcLRC(pSecretKey,*size)) return true;
  else return false;
}


void dumpEEPROM()
{
  Serial.println("Dumping EEPROM");
  for(uint16_t i=0; i < EEPROM.length(); i++)
  {
    Serial.print(EEPROM.read(i));
    Serial.print(" ");
    if ((i>0) && ((i%16) == 0)) Serial.println("");
  }
}