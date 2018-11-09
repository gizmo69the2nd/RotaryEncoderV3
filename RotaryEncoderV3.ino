#include "config.h"
#include "types.h"
#include "callbackFunctions.h"
#include <TaskScheduler.h>
#include "helper_functions.h"

uint8_t state;        //current FSM state
uint8_t index = 0;
uint8_t keySize = MAX_KEY_SIZE;
uint8_t runMode = RUN;

uint64_t startTime = 0;

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
uint8_t fsmOpen();
uint8_t fsmLoadEEPROM();

//Programming specific states
uint8_t fsmReadButton();
uint8_t fsmSaveEEPROM();
uint8_t fsmAddToKey();



//Define task scheduler
Scheduler ts;

Task tInputTimer(INPUT_TIME, TASK_FOREVER, &callbackInput, &ts, false);
Task tResetTimer(RESET_TIME, TASK_FOREVER, &callbackReset, &ts, false);
Task tClearTimer(CLEAR_TIME, TASK_FOREVER, &callbackClear, &ts, false);
Task tReadTimer(READ_INTERVAL, TASK_FOREVER, &callbackRead, &ts, false);
Task tBlinkLED(BLINK_INTERVAL, TASK_FOREVER, &callbackBlink, &ts, false);


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
    /*
    secretKey[0].position = 15;
    secretKey[0].direction = CW;

    secretKey[1].position = 20;
    secretKey[1].direction = CCW;

    secretKey[2].position = 1;
    secretKey[2].direction = CW;

    keySize = 3;
    */
    state = LOAD_EEPROM;

    while (digitalRead(BUTTON) == 0)
    {
        setLED(ORANGE);
        runMode = PROG;
        state = RESET;
        //Serial.println("Entering PROG mode");
    }

}

void loop()
{
    /*
  clearEEPROM();
  dumpEEPROM();
  while(1==1);
  */

  if (runMode == RUN) 
  {
    if (tBlinkLED.isEnabled()) tBlinkLED.disable();
    execNormal();
  }
  else
  {
    if (!tBlinkLED.isEnabled()) tBlinkLED.enable();
    execProg();
  }
}


//Normal execution loop
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
      nextState = fsmOpen();
      break;
    
    case LOAD_EEPROM:
      nextState = fsmLoadEEPROM();
      break;

    default:
    nextState = RESET;
    
  }
  state = nextState;
  ts.execute();
}


//Programming execution loop
void execProg()
{
  //Serial.println("Looping PROG");
  uint8_t nextState = RESET;
  switch(state)
  {
    case RESET:
      nextState = fsmReset();
      break;

    case READ_ENC:
      fsmReadEnc(); //Ignore return value
      nextState = READ_BUTTON; //Manualy set next state
      break;

    case READ_BUTTON:
      nextState = fsmReadButton();
      break;

    case ADD_TO_KEY:
      nextState = fsmAddToKey();
      break;

    case SAVE_EEPROM:
      nextState = fsmSaveEEPROM();
      break;

    case IDLE:
      //Do nothing until READ_INTERVAL timer has run out.
      //Serial.println("Ideling");
      nextState = fsmIdle();
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
 * Check if programming button is pressed.
 * If pressed and released, return ADD_TO_KEY
 * Else go into IDLE mode
 **/
uint8_t fsmReadButton()
{
  //Serial.println("Reading button");
  if (digitalRead(BUTTON) == 0)
  {
    //Button pressed
    //Serial.print("Button pressed ");
    //Serial.println(buttonCounter);
    if (startTime == 0) startTime = millis();
  }
  else
  {
    //Button released
    if (startTime > 0)
    {
      Serial.println("Button released");
      uint64_t timePassed = millis() - startTime;
      if (timePassed >= BUTTON_SHORT)
      {
        startTime = 0;
        if (timePassed >= BUTTON_LONG)
        {
          Serial.println("Button long press");
          return SAVE_EEPROM;
        }
        else
        {
          Serial.println("Button short press");
          return ADD_TO_KEY;
        }
      }
      startTime = 0;
    }
  }
  return IDLE;
}

/**
 * Add current position and direction to the secret key
 * Increase KeySize by one 
 * Return to IDLE
 **/
uint8_t fsmAddToKey()
{
  Serial.print("Adding ");
  Serial.print(currentInput.position);
  Serial.print(" - ");
  Serial.print(currentInput.direction);
  Serial.println(" to secret key...");

  secretKey[index] = currentInput;
  index++;
  printSecretKey(secretKey,index);
  //delay(5000);
  return IDLE;
}

/**
 * Save current secret key to EEPROM
 * Save keySize to EEPROM
 * After saving change mode to RUN and state to RESET 
 * 
 **/
uint8_t fsmSaveEEPROM()
{
  runMode = RUN;
  Serial.println("Saving Secret Key to EEPROM");
  writeEEPROM(secretKey,index);
  //dumpEEPROM();
  return LOAD_EEPROM;
}


/**
 * Load secret key from EEPROM
 * 
 **/
uint8_t fsmLoadEEPROM()
{
  //dumpEEPROM();
  Serial.println("Loading Secret Key from EEPROM");
  if (readEEPROM(secretKey,&keySize))
  {
      //Key loaded correctly
    Serial.println("Loaded Valid key: ");
    printSecretKey(secretKey,keySize);
  }
  else
  {
      //No valid key found. Enter PROG mode
    Serial.println("No Valid Key");
    runMode = PROG;
  }

  
  return RESET;
}

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
    uint8_t ret = IDLE;
    tReadTimer.disable();
    
    uint8_t shiftVal = read_shift();
    if (shiftVal != 0) currentInput = readInputs(prevInput,shiftVal);

    if (currentInput.position != prevInput.position)
    {
      
        Serial.print("prev:\t");
        Serial.print(prevInput.position);
        Serial.print("\t");
        Serial.print(dirToString(prevInput.direction));
        
        Serial.print("\t\tcurr:\t");
        Serial.print(currentInput.position);
        Serial.print("\t");
        Serial.println(dirToString(currentInput.direction));

        ret = RESTART_TIMERS;
    }
    else ret = IDLE;

    prevInput = currentInput;
    return ret;
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
 * else goto IDLE
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
 * goto OPEN state
 **/
uint8_t fsmSolved()
{
    tInputTimer.disable();
    tClearTimer.disable();
    

    Serial.println("Solved puzzle");

    //Turn on relay
    digitalWrite(RELAY,HIGH);

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

uint8_t fsmOpen()
{

  ///start reset timeout 
  if (!tResetTimer.isEnabled()) tResetTimer.enableDelayed();
  
  return OPEN;
}