#include "config.h"
#include "types.h"
#include "callbackFunctions.h"
#include <TaskScheduler.h>
#include "helper_functions.h"

uint8_t state;        //current FSM state
uint8_t index = 0;
uint8_t keySize = MAX_KEY_SIZE;
uint8_t runMode = NORMAL;

InputValue currentInput;
InputValue prevInput;

InputValue secretKey[MAX_KEY_SIZE];

uint8_t fsmReset();
uint8_t fsmReadEnc();
uint8_t fsmCheckPosDir();
uint8_t fsmCheckDirChange();
uint8_t fsmRestartTimer();
uint8_t fsmOK();
uint8_t fsmNOK();
uint8_t fsmSolved();
uint8_t fsmIdle();



//Define task scheduler
Scheduler ts;

Task tInputTimer(INPUT_TIME, TASK_FOREVER, &callbackInput, &ts, false);
Task tResetTimer(RESET_TIME, TASK_FOREVER, &callbackReset, &ts, false);
Task tClearTimer(CLEAR_TIME, TASK_FOREVER, &callbackClear, &ts, false);
Task tReadTimer(READ_INTERVAL, TASK_FOREVER ,&callbackRead, &ts, true);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  /* Initialize digital pins...
  */
  pinMode(LOAD_PIN, OUTPUT);
  pinMode(CLK_EN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  pinMode(RELAY,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(BUTTON,INPUT_PULLUP);

  digitalWrite(CLK_PIN, LOW);
  digitalWrite(LOAD_PIN, HIGH);

  secretKey[0].position = 15;
  secretKey[0].direction = CW;

  secretKey[1].position = 20;
  secretKey[1].direction = CCW;

  secretKey[2].position = 1;
  secretKey[2].direction = CW;

  while (digitalRead(BUTTON) == 0)
  {
    runMode = PROG;
  }

  keySize = 3;

  state = RESET;

}

void loop()
{
  if (runMode == NORMAL) 
  {
    execNormal();
  }
  else
  {
    execProg();
  }
}

void execNormal() {
  // put your main code here, to run repeatedly:
  uint8_t nextState = RESET;
  switch (state)
  {
    case RESET:
      nextState = fsmReset();
      break;

    case READ_ENC:
      nextState = fsmReadEnc();
      break;

    case CHECK_POS_DIR:
      nextState = fsmCheckPosDir();
      break;

    case CHECK_DIR_CHANGE:
      nextState = fsmCheckDirChange();
      break;

    case RESTART_TIMERS:
      nextState = fsmRestartTimer();
      break;

    case OK:
      nextState = fsmOK();
      break;

    case NOK:
      nextState = fsmNOK();
      break;

    case SOLVED:
      nextState = fsmSolved();
      break;

    case IDLE:
      //Do nothing until READ_INTERVAL timer has run out.
      //Serial.println("Ideling");
      nextState = fsmIdle();
      break;

    case OPEN:
      nextState = OPEN;
      break;

    default:
    nextState = RESET;
    
  }
  state = nextState;
  ts.execute();
}


/**
 * FSM functions
 * 
 **/


/**
 * Reset all timers, outputs and variables
 * Next state is READ_ENC
 **/

uint8_t fsmReset()
{
    Serial.println("Resetting");
    //Reset index to 0
    index = 0;

    uint8_t shiftVal = read_shift();
    if (shiftVal != 0) prevInput = readInputs(prevInput,shiftVal);
    prevInput.direction = STOP;

    //Turn LED off
    setLED(OFF);

    //disable input timer
    tInputTimer.disable();
   
    //disable clear timer 
    tClearTimer.disable();

    //disable reset timer
    tResetTimer.disable();

    //restart read timer
    if (tReadTimer.isEnabled()) tReadTimer.restartDelayed();
    else tReadTimer.enableDelayed();

    //release relay
    digitalWrite(RELAY,LOW);

    return IDLE;
}

/**
 * Read encoder value
 * If the value changed goto RESTART_TIMERS state
 * Else state remains READ_ENC
 **/
uint8_t fsmReadEnc()
{
    tReadTimer.disable();
    prevInput = currentInput;
    uint8_t shiftVal = read_shift();
    if (shiftVal != 0) currentInput = readInputs(prevInput,shiftVal);
    /*
    Serial.print("prev:\t");
    Serial.print(prevInput.position);
    Serial.print("\t");
    Serial.println(dirToString(prevInput.direction));
    
    Serial.print("curr:\t");
    Serial.print(currentInput.position);
    Serial.print("\t");
    Serial.println(dirToString(currentInput.direction));
*/
    if (currentInput.position != prevInput.position) return RESTART_TIMERS;
    else return IDLE;
}

/**
 * Compare current positional value and direction to secret key values
 * This is called after encoder remains in the same position for INPUT_TIME ms
 * If position and direction equals current key index goto OK state
 * Else goto NOK state
 **/
uint8_t fsmCheckPosDir()
{
    Serial.println("Checking input timeout");

    tInputTimer.disable();

    if (checkInput(currentInput,index,secretKey)) return OK;
    else return NOK;
}

/**
 * Check if the direction has changed
 * If direction changed compare previous input to secret key
 *     If they are equal goto OK
 *     Else goto NOK
 * Else goto READ_ENC
 * 
 * -------!!! DETECTING DIRECTION CHANGE HAS BEEN DISABLED !!------
 * Only checking on timeout
 * 
 * 
 **/
uint8_t fsmCheckDirChange()
{
    if ((prevInput.direction != currentInput.direction) && (prevInput.direction != STOP))
    {
      //Serial.println("prev: " + dirToString(prevInput.direction) + "\tcur: " + dirToString(currentInput.direction));
      Serial.println("Direction Changed");
      if (checkInput(prevInput,index,secretKey)) return OK;
      else return NOK;
    }
    else return IDLE;
}

/**
 * Every time the input changes:
 * Restart INPUT_TIME timer
 * Restart CLEAR_TIME timer
 * Restart RESET_TIME timer
 * Turn off LED
 **/
uint8_t fsmRestartTimer()
{
    Serial.println("Restarting timers");

    //Turn LED off
    setLED(OFF);

    //restart input timeout
    if (tInputTimer.isEnabled())
    {
       tInputTimer.restartDelayed();
       //Serial.println("input timer was enabled, restarting");
    }
    else 
    {
      tInputTimer.enableDelayed();
      //Serial.println("input timer was disabled, enabeling");
    }

    //restart clear timeout 
    if (tClearTimer.isEnabled()) tClearTimer.restartDelayed();
    else tClearTimer.enableDelayed();

    return IDLE;
}

/**
 * Called when current position and direction match secret key
 * Increase current index by one and light up green LED
 * If index equals KeySize goto SOLVED
 * else goto READ_ENC
 **/
uint8_t fsmOK()
{
    Serial.println("Input OK");

    setLED(GREEN);
    index++;
    Serial.println("Index: " + String(index));
    if (index >= keySize) return SOLVED;
    else return IDLE;
}

/**
 * Called when current position and direction don't match secret key
 * Or CLEAR_TIME timer has expired
 * 
 * reset index to zero and light up red LED
 **/
uint8_t fsmNOK()
{
    Serial.println("Input not OK");
    
    tClearTimer.disable();
    setLED(RED);
    index = 0;

    Serial.println("Index: " + String(index));
    return IDLE;
}

/**
 * Called when index matches KeySize. This means correct sequence is entered.
 * Toggle relay
 * restart RESET_TIME timer
 * goto IDLE state
 **/
uint8_t fsmSolved()
{
    tInputTimer.disable();
    tClearTimer.disable();
    

    Serial.println("Solved puzzle");

    //Turn on relay
    digitalWrite(RELAY,HIGH);

    //start reset timeout 
    if (tResetTimer.isEnabled()) tResetTimer.restartDelayed();
    else tResetTimer.enableDelayed();

    return OPEN;
}

uint8_t fsmIdle()
{
  //restart read timer
  if (!tReadTimer.isEnabled())
  {
    //Serial.println("restarting read timer");
    tReadTimer.enableDelayed();
  }

  return IDLE;
}