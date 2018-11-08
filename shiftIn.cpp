#include "shiftIn.h"

uint8_t read_shift()
{
    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(CLK_EN, HIGH);
    digitalWrite(LOAD_PIN, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(LOAD_PIN, HIGH);
    digitalWrite(CLK_EN, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(int i = DATA_WIDTH; i >0; i--)
    {      
        if (digitalRead(DATA_PIN)) return i;

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(CLK_PIN, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(CLK_PIN, LOW);
    }

    return 0;
}
