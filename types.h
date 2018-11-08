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
  OFF
}Led_Color;

typedef struct 
{
  uint8_t position;
  Direction direction;
}InputValue;

typedef enum
{
  NORMAL,
  PROG
}RunMode;

#endif
