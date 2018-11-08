#include "fsmFunctions.h"




/**
 * Reset all timers, outputs and variables
 * Next state is READ_ENC
 **/
uint8_t fsmReset()
{
    Serial.println("Resetting");
    //Reset index to 0
    index = 0;

    prevInput = readInputs(prevInput);
    prevInput.direction = STOP;

    //Turn LED off
    setLED(OFF);

    //disable input timeout
    //tInputTimer.disable();
   
    //disable reset timeout 
    //tClearTimer.disable();

    //release relay
    digitalWrite(RELAY,LOW);

    return READ_ENC;
}

/**
 * Read encoder value
 * If the value changed goto RESTART_TIMERS state
 * Else state remains READ_ENC
 **/
uint8_t fsmReadEnc()
{
    currentInput = readInputs(prevInput);
    if (currentInput.position != prevInput.position) return RESTART_TIMERS;
    else return READ_ENC;
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

    if (checkInput(currentInput,index,secretKey)) return OK;
    else return NOK;
}

/**
 * Check if the direction has changed
 * If direction changed compare previous input to secret key
 *     If they are equal goto OK
 *     Else goto NOK
 * Else goto READ_ENC
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
    else return READ_ENC;
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
/*
    //restart input timeout
    if (tInputTimer.isEnabled()) tInputTimer.restartDelayed();
    else tInputTimer.enableDelayed();

    //restart clear timeout 
    if (tClearTimer.isEnabled()) tClearTimer.restartDelayed();
    else tClearTimer.enableDelayed();
*/
    return CHECK_DIR_CHANGE;
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

    if (index >= keySize) return SOLVED;
    else return READ_ENC;
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

    setLED(RED);
    index = 0;
    return READ_ENC;
}

/**
 * Called when index matches KeySize. This means correct sequence is entered.
 * Toggle relay
 * restart RESET_TIME timer
 * goto IDLE state
 **/
uint8_t fsmSolved()
{
    Serial.println("Solved puzzle");

    //Turn on relay
    digitalWrite(RELAY,HIGH);
/*
    //start reset timeout 
    if (tResetTimer.isEnabled()) tResetTimer.restartDelayed();
    else tResetTimer.enableDelayed();
*/
    return IDLE;
}