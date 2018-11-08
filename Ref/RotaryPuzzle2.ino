/*----==== Rotary Puzzle ====----
* Created by Davy Van Belle
* 
* This puzzle takes 24 inputs from a rotary switch.
* 3x 8-bit parallel to serial shit register are used.
*
* When the user inputs the correct code in the correct sequence a relay output turns on for 'OPEN_TIMEOUT' seconds
* The user need to turn the wheel in the correct direction as part of the code.
* If the direction is changed, the last value will be used as input
* Whenever the input timer raises a INPUT_TIMEOUT event, the current value will be used as input
* Whenever the reset timer raises a RESET_TIMEOUT event, the list with input values is cleared and the game is reset.
*
* If the user inputs a wrong value, the list with input values is cleared and the game is reset.
*
* When the admin hold the PROG button while powering the system, the run mode will change to programming mode
*
* If the PROG button is pressed in programming mode, the list of input values is saved to the EEPROM.
*
* A green/red LED will show the user if the last value was correct or not.
*/

//#define SIM
#define DEBUG

#include <Arduino.h>
#include <TaskScheduler.h>
#include "config.h"
#include "types.h"
#include "helper_functions.h"
#include "shiftIn.h"

uint8_t state = TEST;
RunMode mode = NORMAL;

InputValue currentInput = {0,STOP};
InputValue prevInput = {0,STOP};

InputValue secretKey[MAX_KEY_SIZE];

uint8_t index = 0;
uint8_t keySize = MAX_KEY_SIZE;

void callbackInput();
void callbackReset();
void callbackUnlock();

//Define task scheduler
Scheduler ts;

Task tInput(INPUT_TIMEOUT, TASK_FOREVER, &callbackInput, &ts, false);
Task tReset(RESET_TIMEOUT, TASK_FOREVER, &callbackReset, &ts, false);
Task tUnlock(UNLOCK_TIMEOUT, TASK_FOREVER, &callbackUnlock, &ts, false);


#ifdef DEBUG
#include "debug.h"
#endif

#ifdef SIM
#include "simulate.h"
Task tTurn(turnTime, TASK_FOREVER, &callbackTurn, &ts, true);
unsigned long turnTime;
uint8_t *prevPos = &prevInput.position;
uint8_t inShift = 0;
#endif

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

    /* Initialize our digital pins...
  */
  pinMode(LOAD_PIN, OUTPUT);
  pinMode(CLK_EN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  digitalWrite(CLK_PIN, LOW);
  digitalWrite(LOAD_PIN, HIGH);

  secretKey[0].position = 15;
  secretKey[0].direction = CW;

  secretKey[1].position = 20;
  secretKey[1].direction = CW;

  keySize = 2;
  
    //INIT
    //--> Read code from EEPROM, init port direction, init timers, init variables

  #ifdef SIM

  randomSeed(analogRead(0));
  
  currentInput.direction = CW;
  currentInput.position = 10;

  prevInput = currentInput;

  secretKey[0].position = 15;
  secretKey[0].direction = CW;

  secretKey[1].position = 20;
  secretKey[1].direction = CW;

  keySize = 2;
  #endif //SIM//
}

#ifdef SIM
void callbackTurn()
{
  #ifdef DEBUG
  Serial.println("Turn callback:");
  #endif
  inShift = randomRot();
}
#endif


void callbackInput()
{
  #ifdef DEBUG
  Serial.println("input callback");
  #endif
  state = INPUT_TO;
}

void callbackReset()
{
  #ifdef DEBUG
  Serial.println("reset callback");
  #endif
  state = RESET;
}

void callbackUnlock()
{
  #ifdef DEBUG
  Serial.println("unlock callback");
  #endif
  if (tUnlock.isEnabled()) tUnlock.disable();
  prevInput = {0,STOP};
  currentInput = {0,STOP};
  state = RESET;
}


//Finite state machine functions
//Each function returns the next FSM state

uint8_t fsmTest(uint8_t c_state)
{
  #ifdef DEBUG
  
  Serial.print("prev:\t");
  Serial.print(prevInput.position);
  Serial.print("\t");
  Serial.println(dirToString(prevInput.direction));
  
  Serial.print("curr:\t");
  Serial.print(currentInput.position);
  Serial.print("\t");
  Serial.println(dirToString(currentInput.direction));

  Serial.print("Index:\t");
  Serial.println(index);
  //Serial.println("");
  #endif
  delay(250);
  return ANALYSE;
}

uint8_t fsmRead(uint8_t c_state)
{
  //READ INPUT
  prevInput = currentInput;
  #ifndef SIM
  uint8_t shiftVal = read_shift();
  if (shiftVal != 0) currentInput = readInputs(prevInput,shiftVal);
  #else
  currentInput = simInputs(prevInput,inShift);
  #endif

  return TEST;
}

uint8_t fsmAnalyse(uint8_t c_state)
{
  //ANALYSE INPUT
  if (currentInput.position != prevInput.position)
  {
    //Simulate input behavior
    #ifdef SIM
    //turnTime =turnTime + (random(-5,5)*100);
    //if (turnTime < 500 ) turnTime = 2500;

    if ((currentInput.position == 15) || (currentInput.position == 20)) turnTime = 3000;
    else turnTime = TURN_TIMEOUT;

    #ifdef DEBUG
    Serial.println("restart timer: " + String(turnTime));
    #endif
    tTurn.setInterval(turnTime);
    #endif

    //restart input timeout
    if (tInput.isEnabled()) tInput.restartDelayed();
    else tInput.enableDelayed();

    //restart reset timeout 
    if (tReset.isEnabled()) tReset.restartDelayed();
    else tReset.enableDelayed();

    //LED = CLEAR
    setLED(OFF);

    if ((prevInput.direction != currentInput.direction) && (prevInput.direction != STOP))
    {
      #ifdef DEBUG
      Serial.println("prev: " + dirToString(prevInput.direction) + "\tcur: " + dirToString(currentInput.direction));
      #endif
      Serial.println("Index before call: " + String(index));
      return checkInput(prevInput,&index,mode,secretKey,keySize);
      Serial.println("Index after call: " + String(index));
    }
  }

  return READ;
}

uint8_t fsmUnlock(uint8_t c_state)
{
  //UNLOCK
  #ifdef DEBUG
  Serial.println("Secret found opening lock");
  #endif

  //relay output = HIGH

  //Stop turn task
  #ifdef SIM
  if (tTurn.isEnabled()) tTurn.disable();
  #endif

  //stop input task
  if (tInput.isEnabled()) tInput.disable();
  //stop reset task
  if (tReset.isEnabled()) tReset.disable();
  //start unlock task
  if (!tUnlock.isEnabled()) tUnlock.enableDelayed();
  
  return WAIT;
}

uint8_t fsmInputTo(uint8_t c_state)
{
  
  if (tInput.isEnabled()) tInput.disable();
  //INPUT TIMEOUT
  Serial.println("Index before call: " + String(index));
  uint8_t t = checkInput(currentInput,&index,mode,secretKey,keySize);
  Serial.println("Index after call: " + String(index));
  return t;
}

uint8_t fsmReset(uint8_t c_state)
{
  //Reset timers
  if (tReset.isEnabled()) tReset.disable();
  if (tUnlock.isEnabled()) tUnlock.disable();
  if (tInput.isEnabled()) tInput.disable();
  
  #ifdef SIM
  if (!tTurn.isEnabled()) tTurn.enable();
  #endif

  #ifdef DEBUG
  Serial.println("Closing lock");
  #endif

  index = 0;

  //prevInput = {0,STOP};
  //currentInput = {0,STOP};

  return READ;
}





void loop()
{
  //Serial.println(state);
  uint8_t nextState;
  switch(state)
  {
    case READ:
      nextState = fsmRead(state);
      break;

    case ANALYSE:
      nextState = fsmAnalyse(state);
      break;

    case UNLOCK:
      nextState = fsmUnlock(state);
      break;

    case INPUT_TO:
      nextState = fsmInputTo(state);
      break;

    case RESET:
      nextState = fsmReset(state);
      break;

    case TEST:
      nextState = fsmTest(state);
      break;

    case WAIT:
      break;

    default:
      state = READ;
      break;
  }

  state = nextState;
  ts.execute();
}
