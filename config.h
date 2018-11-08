#ifndef CONFIG_H
#define CONFIG_H

//Pins
#define LOAD_PIN  5  // Connects to Parallel load pin the 165
#define CLK_EN    2  // Connects to Clock Enable pin the 165
#define DATA_PIN  6 // Connects to the Q7 pin the 165
#define CLK_PIN   4 // Connects to the Clock pin the 165

#define RELAY     13
#define RED_LED   10
#define GREEN_LED 11
#define BUTTON    9


//Timer values
#define INPUT_TIME          1500
#define CLEAR_TIME          10000
#define RESET_TIME          5000
#define READ_INTERVAL       111

#define MAX_KEY_SIZE        5

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
