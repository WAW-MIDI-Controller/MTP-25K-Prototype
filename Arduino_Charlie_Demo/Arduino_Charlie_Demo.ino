/**********************************************************************************
* WAW MIDI MTP-25K
* Arduino Charlie Demo
* Dec 2025
* by Walker Bradley
*
* This program is designed to demonstrate the MTP-25K's charlieplexing array. Due 
* to hardware challenges with the Teensy 4.1 microcontroller, this code is modified 
* to run on an Arduino Uno.
* 
* To adjust the displayed pattern, change the values in the LEFT_HAND and 
* RIGHT_HAND arrays. Use MIDI note values for notes, and REST for rests.
*
* To adjust playback speed, adjust the TEMPO definition.
* 
* *Note* Right hand harmonization intervals are handled in the main loop for this 
*        pattern.
**********************************************************************************/

/********************************** LIBRARIES ************************************/
#include <Arduino.h>
#include <Adafruit_IS31FL3731.h>
/*********************************************************************************/

/*************************** CHARLIEPLEXING VARIABLES ****************************/
// Charlieplexing driver instance
Adafruit_IS31FL3731 led_Matrix = Adafruit_IS31FL3731();

// Charlieplexing addresses
const uint8_t TOUCHPAD_LEDS_Y[] = {
  3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6};
const uint8_t TOUCHPAD_LEDS_X[] = {
  0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0};

const uint8_t OCT_LEDS_Y[] = {2,2,2,2,2,2};
const uint8_t OCT_LEDS_X[] = {1,2,3,4,5,6};

const uint8_t SET_NUM_LEDS_Y[] = {0,0,0,2,1,3};
const uint8_t SET_NUM_LEDS_X[] = {8,9,10,9,9,9};

const uint8_t ARP_PATTERN_LEDS_Y[] = {6,6,6,6};
const uint8_t ARP_PATTERN_LEDS_X[] = {7,6,5,4};

const uint8_t NOTE_LENGTH_LEDS_Y[] = {1,2,3};
const uint8_t NOTE_LENGTH_LEDS_X[] = {8,8,8};

const uint8_t HOLD_ALL_LED_Y = 6;
const uint8_t HOLD_ALL_LED_X = 2;

const uint8_t HOLD_SET_LED_Y = 6;
const uint8_t HOLD_SET_LED_X = 3;

const uint8_t HOLD_LED_Y = 6;
const uint8_t HOLD_LED_X = 2;

const uint8_t ARP_LED_Y = 6;
const uint8_t ARP_LED_X = 1;

const uint8_t TEMPO_LED_Y = 2;
const uint8_t TEMPO_LED_X = 0;

const uint8_t SHIFT_LED_Y = 2;
const uint8_t SHIFT_LED_X = 7;
/*********************************************************************************/

/********************************** DEFINITIONS **********************************/
// Musical definitions
#define USB_CHANNEL 1       // MIDI channel used
#define OCTAVE 12           // Notes in an octave
#define OCTAVE_NUM 0        // Hardset octave number
#define C1 24               // MIDI value for C1
#define REST 255            // Non-MIDI value representing rests  

// Tempo interval calculation definitions
#define BPM_CONV 30000000   // BPM -> microsecond intervals
#define TEMPO 120

// Total notes in play patterns definition
#define MAX_SET_NOTES 96

// LED brightness definition
#define BRIGHTNESS 100

// Beats per quarter note definition
#define QUARTER 1
#define EIGHTH 2
#define SIXTEENTH 4
/*********************************************************************************/

/***************************** CALCULATED CONSTANTS ******************************/
// Calculated tempo interval
const long TEMPO_INTERVAL = (float)BPM_CONV/float(TEMPO * EIGHTH);

// Octave note motifier
const uint8_t OCTAVE_SHIFT = OCTAVE * OCTAVE_NUM;
/*********************************************************************************/

/********************************* PLAY PATTERNS *********************************/
// Left hand pattern
const uint8_t LEFT_HAND[] = { 
   48 ,REST, 52 ,REST, 55 ,REST, 57 ,REST, 58 ,REST, 57 ,REST, 55 ,REST, 52 ,REST,
   48 ,REST, 52 ,REST, 55 ,REST, 57 ,REST, 58 ,REST, 57 ,REST, 55 ,REST, 52 ,REST,
   53 ,REST, 57 ,REST, 60 ,REST, 62 ,REST, 63 ,REST, 62 ,REST, 60 ,REST, 57 ,REST,
   48, REST, 52 ,REST, 55 ,REST, 57 ,REST, 58 ,REST, 57 ,REST, 55 ,REST, 52 ,REST,
   55 ,REST, 59 ,REST, 62 ,REST, 59 ,REST, 53 ,REST, 57 ,REST, 60 ,REST, 57 ,REST,
   48 ,REST, 52 ,REST, 55 ,REST, 57 ,REST, 58 ,REST, 57 ,REST, 55 ,REST, 52 ,REST
};

// Right hand pattern
const uint8_t RIGHT_HAND[] = {
  REST, 64 ,REST,REST, 64 ,REST,REST,REST, 64 ,REST,REST, 64 ,REST,REST,REST,REST,
  REST, 64 ,REST,REST, 64 ,REST,REST,REST, 64 ,REST,REST, 64 ,REST,REST,REST,REST,
  REST, 65 ,REST,REST, 65 ,REST,REST,REST, 65 ,REST,REST, 65 ,REST,REST,REST,REST,
  REST, 64 ,REST,REST, 64 ,REST,REST,REST, 64 ,REST,REST, 64 ,REST,REST,REST,REST,
  REST, 67 ,REST,REST, 67 ,REST,REST,REST, 65 ,REST,REST, 65 ,REST,REST,REST,REST,
  REST, 64 ,REST,REST, 64 ,REST,REST,REST, 64 ,REST,REST, 64 ,REST,REST,REST,REST
};
/*********************************************************************************/

/******************************* GLOBAL VARIABLES ********************************/
// Signal to turn adjust LEDs
volatile uint8_t _adjust_LEDs = 0;

/* 8 element array of 16 bit ints used to track active notes
********************************************************************************
Octave (Element  N/A N/A N/A N/A  B   A#  A   G#  F   F#  F   E   D#  D   C#  C   
of array)        MSB                                                         LSB
C1-B1 - [0]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C2-B2 - [1]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C3-B3 - [2]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C4-B4 - [3]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C5-B5 - [4]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C6-B6 - [5]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C7-B7 - [6]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
C8    - [7]       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
********************************************************************************
*/
volatile uint16_t _active_notes[] = {0,0,0,0,0,0,0,0};
/*********************************************************************************/

/****************************** FUNCTION PROTOTYPES ******************************/
void startNote(uint8_t note, uint8_t velocity);
void stopNote(uint8_t note);
void adjustKeyLEDs();
/*********************************************************************************/


/************************************* SETUP *************************************/
void setup() {
  Serial.begin(9600);

  Serial.println("Initializing...");

  // Connect LED matrix
  if(!led_Matrix.begin(0x74,&Wire))
  {
    Serial.println("Error with IS31");
  } else Serial.println("IS31 Up!");


  // Status update
  Serial.println("Setting LEDs");

  //Initialize setting LEDs
  led_Matrix.drawPixel(OCT_LEDS_X[2], OCT_LEDS_Y[2], BRIGHTNESS);
  led_Matrix.drawPixel(SET_NUM_LEDS_X[3], SET_NUM_LEDS_Y[3], BRIGHTNESS);
  led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[0], ARP_PATTERN_LEDS_Y[0], BRIGHTNESS);
  led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[1], NOTE_LENGTH_LEDS_Y[1], BRIGHTNESS);

  // Status update
  Serial.println("Starting loop.");
}
/*********************************************************************************/

/*********************************** MAIN LOOP ***********************************/
void loop() {
  // Persistent variables
  static uint8_t play_position = 0;
  static uint8_t previous_right_note = 0;
  static long current_micros = 0;
  static long exit_micros = 0;
 
  current_micros = micros();
  if((current_micros - exit_micros) >= TEMPO_INTERVAL)
  {
    // Control TEMPO_LED
    if((play_position % 4) == 0)
    {
      led_Matrix.drawPixel(TEMPO_LED_X, TEMPO_LED_Y, BRIGHTNESS);
    }
    else if((play_position % 4) == 1)
    {
      led_Matrix.drawPixel(TEMPO_LED_X, TEMPO_LED_Y, 0);
    }

    // Change left hand pattern's note/LED state
    if(LEFT_HAND[play_position] != REST)
    {
      startNote(LEFT_HAND[play_position] + OCTAVE_SHIFT, 99);
    }
    else if(LEFT_HAND[play_position] == REST)
    {
      stopNote(LEFT_HAND[play_position - 1] + OCTAVE_SHIFT);
    }

    // Change right hand pattern's note/LED state
    if(RIGHT_HAND[play_position] != REST)
    {
      startNote(RIGHT_HAND[play_position] + OCTAVE_SHIFT, 99);

      // Adjust harmonizing interval
      if(RIGHT_HAND[play_position] == 64) 
        startNote(RIGHT_HAND[play_position] + 3 + OCTAVE_SHIFT, 99);
      else 
        startNote(RIGHT_HAND[play_position] + 4 + OCTAVE_SHIFT, 99);

      // Store note for readability
      previous_right_note = RIGHT_HAND[play_position];
    }
    else if(RIGHT_HAND[play_position] == REST)
    {
      stopNote(previous_right_note + OCTAVE_SHIFT);
      if(previous_right_note == 64)
        stopNote(previous_right_note + 3 + OCTAVE_SHIFT);
      else
        stopNote(previous_right_note + 4 + OCTAVE_SHIFT);
    }
    
    // If a note state changed, adjust the LEDs
    if(_adjust_LEDs)
    {
      adjustKeyLEDs();
    }

    // Increment play_position, or restart it at the end of the pattern
    play_position = (play_position + 1) % MAX_SET_NOTES;

    // Track exit time
    exit_micros = micros();
  }
}
/*********************************************************************************/

/*********************************** FUNCTIONS ***********************************/

// Adjusts active note flag, and signals that LEDs need to be adjusted
void startNote(uint8_t note, uint8_t velocity)
{
  // MIDI not compatible with Arduino
  //usbMIDI.sendNoteOn(note, velocity, channel);
  if(!(_active_notes[(note - C1) / OCTAVE] & (1 << (note % OCTAVE))))
  {
    _active_notes[(note - C1) / OCTAVE] += (1 << (note % OCTAVE));
  }
  _adjust_LEDs = 1;
}

/*********************************************************************************/

// Adjusts active note flag, and signals that LEDs need to be adjusted
void stopNote(uint8_t note)
{
  // MIDI not compatible with Arduino
  //usbMIDI.sendNoteOff(note, 0, channel);
  if(_active_notes[(note - C1) / OCTAVE] & (1 << (note % OCTAVE)))
  {
     _active_notes[(note - C1) / OCTAVE] -= (1 << (note % OCTAVE));
  }
  _adjust_LEDs = 1;
}

/*********************************************************************************/

// Set LEDs for keybed
void adjustKeyLEDs()
{
  for(uint8_t i = 0; i < 13; i++)
  {
    uint16_t bit_mask = 1 << (i % OCTAVE);
    // Left side
    if(_active_notes[2 + OCTAVE_NUM + i / OCTAVE] & bit_mask)
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[i], TOUCHPAD_LEDS_Y[i], BRIGHTNESS);
    }
    else
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[i], TOUCHPAD_LEDS_Y[i], 0);
    }
    // Right side
    if(_active_notes[3 + OCTAVE_NUM + i / OCTAVE] & bit_mask)
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[12 + i], TOUCHPAD_LEDS_Y[12 + i], BRIGHTNESS);
    }
    else
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[12 + i], TOUCHPAD_LEDS_Y[12 + i], 0);
    }
  }
  _adjust_LEDs = 0;
}
/*********************************************************************************/
/******************************* END PROGRAM *************************************/
/*********************************************************************************/