#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

//FSM states
#define RESET               1
#define READ_ENC            2
#define CHECK_POS_DIR       3
#define CHECK_DIR_CHANGE    4
#define OK                  5
#define NOK                 6
#define RESTART_TIMERS      7
#define SOLVED              8
#define IDLE                9
#define OPEN                10
#define READ_BUTTON         11
#define ADD_TO_KEY          12
#define SAVE_EEPROM         13
#define LOAD_EEPROM         14

typedef enum 
{
  STOP,
  CW,
  CCW  
}Direction;

typedef enum
{
  RED,
  GREEN,
  ORANGE,
  OFF
}Led_Color;

typedef struct 
{
  uint8_t position;
  Direction direction;
}InputValue;

typedef enum
{
  RUN,
  PROG
}RunMode;

#endif
