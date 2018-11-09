#ifndef CONFIG_H
#define CONFIG_H

//Pins
#define LOAD_PIN  5  // Connects to Parallel load pin the 165
#define CLK_EN    2  // Connects to Clock Enable pin the 165
#define DATA_PIN  6 // Connects to the Q7 pin the 165
#define CLK_PIN   4 // Connects to the Clock pin the 165

#define RELAY     3
#define RED_LED   7
#define GREEN_LED 8
#define BUTTON    9


//Timer values
#define INPUT_TIME          1500
#define CLEAR_TIME          10000
#define RESET_TIME          5000
#define READ_INTERVAL       12
#define BLINK_INTERVAL      500

//BUTTON debounce
#define BUTTON_SHORT 50
#define BUTTON_LONG  1000


#define MAX_KEY_SIZE        10

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   3

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

#endif
