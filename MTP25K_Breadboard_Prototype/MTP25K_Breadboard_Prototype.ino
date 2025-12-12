/**********************************************************************************
* WAW MIDI MTP-25K
* Breadboard Prototype
* Dec 2025
* by Walker Bradley
*
* This program powers the MTP-25K breadboard prototype. It is designed for the
* Teensy 4.1 Microcontroller by PJRC.
* 
* We are working towards open-source, easily adjusted firmware for hobbyists and
* electronically inclined musicians. Make sure to check back for future updates!
**********************************************************************************/

/********************************** LIBRARIES ************************************/
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_IS31FL3731.h>
#include <MIDI.h>
/*********************************************************************************/

/********************************** BUTTON PINS **********************************/
const uint8_t OCT_DOWN_PIN = 33;
const uint8_t OCT_UP_PIN = 34;
const uint8_t SHIFT_PIN = 35;
const uint8_t HOLD_PIN = 36;
const uint8_t ARP_PIN = 37;
/*********************************************************************************/

/*************************** CHARLIEPLEXING VARIABLES ****************************/
// Charlieplexing addresses
const u_int8_t TOUCHPAD_LEDS_X[] = {
  7,6,5,4,3,2,1,0,8,9,10,11,7,6,5,4,3,2,1,0,8,9,10,11,7};
const u_int8_t TOUCHPAD_LEDS_Y[] = {
  0,1,0,1,0,0,1,0,7,8,7,8,2,3,2,3,2,2,3,2,5,6,5,6,4};

const uint8_t OCT_LEDS_X[] = {7,6,5,4,3,2};
const uint8_t OCT_LEDS_Y[] = {8,8,8,8,8,8};

const uint8_t SET_NUM_LEDS_X[] = {7,6,5,4,3,2};
const uint8_t SET_NUM_LEDS_Y[] = {7,7,7,7,7,7};

const uint8_t ARP_PATTERN_LEDS_X[] = {7,6,5,4,3,2};
const uint8_t ARP_PATTERN_LEDS_Y[] = {6,6,6,6,6,6};

const uint8_t NOTE_LENGTH_LEDS_X[] = {7,6,5,4,3,2};
const uint8_t NOTE_LENGTH_LEDS_Y[] = {5,5,5,5,5,5};

const uint8_t HOLD_ALL_LED_X = 13;
const uint8_t HOLD_ALL_LED_Y = 0;

const uint8_t HOLD_SET_LED_X = 13;
const uint8_t HOLD_SET_LED_Y = 1;

const uint8_t ARP_LED_X = 14;
const uint8_t ARP_LED_Y = 0;

const uint8_t TEMPO_LED_X = 15;
const uint8_t TEMPO_LED_Y = 0;

const uint8_t SHIFT_LED_X = 12;
const uint8_t SHIFT_LED_Y = 0;

const uint8_t DIFFICULT_LED_X = 0;
const uint8_t DIFFICULT_LED_Y = 1;
/*********************************************************************************/

/********************************** DEFINITIONS **********************************/
// Musical definitions
#define USB_CHANNEL 1       // MIDI channel for USB
#define TRS_CHANNEL 2       // MIDI channel for TRS
#define OCTAVE 12           // Notes in an octave
#define C1 24               // MIDI value for C1
#define C2 36               //      "         C2
#define C3 48               //      "         C3
#define C4 60               //      "         C4

// Time interval definitions in microseconds
#define KEY_TIME_US 500     // Time between key checks
#define DEBOUNCE_MS 500     // Button debounce delay
#define BPM_CONV 30000000   // BPM -> microsecond intervals

// Maximum selectable set number definition
#define MAX_SET_NOTES 9

// LED brightness definition
#define BRIGHTNESS 100

// ARP pattern definitions
#define ARP_UP 0
#define ARP_DOWN 1
#define ARP_UP_DOWN 2
#define ARP_CUSTOM 3

// Feature definitions
#define STANDARD 0
#define ARP 1
#define HOLD 2
#define ARP_HOLD 3          // ARP + HOLD

// HOLD sub-mode definitions
#define HOLD_ALL 0
#define HOLD_SET 1

// Quarter note per measure definition
#define ARP_TIME_DIVIDER 4

// Beats per quarter note definitions
#define QUARTER 1
#define EIGHTH 2
#define SIXTEENTH 4

// MCP definitions
#define MCP1_PIN_COUNT 12
#define MCP2_PIN_COUNT 13
/*********************************************************************************/

/************************************ STRUCTS ************************************/
volatile typedef struct {
  uint8_t notes[MAX_SET_NOTES];
  uint8_t velocities[MAX_SET_NOTES];
  uint8_t write_index;
  uint8_t read_index;
  uint8_t note_count;
  uint8_t pattern; // 0 = ARP_UP, 1 = ARP_DOWN, 2 = ARP_UP_DOWN, 3 = ARP_CUSTOM
  uint8_t set_select;
  uint8_t note_length;
} Arp_Buffer;

volatile typedef struct{
  uint8_t notes[MAX_SET_NOTES];
  uint8_t note_count;
  uint8_t set_select;
} Hold_Buffer;
/*********************************************************************************/

/******************************* OBJECT INSTANCES ********************************/
// Arpeggio timer
IntervalTimer arp_Timer;

// Buffers
Arp_Buffer arp_Buffer;
Hold_Buffer hold_Buffer;

// Hardware
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;
Adafruit_IS31FL3731 led_Matrix = Adafruit_IS31FL3731();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, TRS);
/*********************************************************************************/

/******************************* GLOBAL VARIABLES ********************************/
volatile int8_t _octave_num = 0;            // +/- octave selection
volatile uint8_t _set_select = 4;           // SET number selection
volatile uint8_t _feature_select = 0;       // N/A N/A N/A N/A N/A N/A HOLD ARP
volatile uint8_t _shift_on = 0;             // SHIFT on/off flag
volatile uint8_t _hold_mode = 0;            // 0 for hold all, 1 for hold set
volatile uint16_t _tempo = 120;             // TEMPO selection
volatile uint8_t _arp_reset = 0;            // Flag to reinitialize arp buffer
volatile uint8_t _turn_notes_off = 0;       // Flag to turn off all active notes
volatile uint8_t _adjust_key_LEDs = 0;      // Flag to adjust keybed LEDs

/* 8 element array of 16 bit short ints used to track active notes
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
void checkKeys();
void startNote(uint8_t note, uint8_t velocity);
void stopNote(uint8_t note);
void adjustKeyLEDs();
void turnNotesOff();
void octaveUpInterrupt();
void octaveDownInterrupt();
void setSelectUp();
void setSelectDown();
void holdInterrupt();
void arpInterrupt();
void shiftInterrupt();
void arpHandler();
void arpBufferInit(Arp_Buffer* buffer);
void arpBufferReInit(Arp_Buffer* buffer);
void arpBufferNoteLength(Arp_Buffer* buffer);
void arpBufferCyclePattern(Arp_Buffer* buffer);
void arpBufferSort(Arp_Buffer* buffer);
uint8_t arpBufferAdd(Arp_Buffer* buffer, uint8_t note, uint8_t velocity);
uint8_t arpBufferRemove(Arp_Buffer* buffer, uint8_t note);
uint8_t arpBufferIncludes(Arp_Buffer* buffer, uint8_t note);
uint8_t arpBufferGet(Arp_Buffer* buffer, uint8_t position, uint8_t* note, uint8_t* velocity);
void holdBufferInit(Hold_Buffer* buffer);
uint8_t holdBufferIncludes(Hold_Buffer* buffer, uint8_t note);
uint8_t holdBufferAdd(Hold_Buffer* buffer, uint8_t note);
/*********************************************************************************/

/************************************* SETUP *************************************/
void setup() {
  Serial.begin(115200);

  Serial.println("Initializing...");

  // Initialize the buffers
  arpBufferInit(&arp_Buffer);
  holdBufferInit(&hold_Buffer);
  
  // Connect LED matrix
  if(!led_Matrix.begin(0x74,&Wire2))
  {
    Serial.println("Error with IS31");
  } else Serial.println("IS31 Up!");
  
    // Loading bar
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(6, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(5, 3+i, BRIGHTNESS);
    }
    delay(200);

  // Connect MCP1
  if(!mcp1.begin_I2C(0x20,&Wire2))
  {
    Serial.println("Error with MCP1");
  } else Serial.println("MCP1 Up!");

    // Loading bar  
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(4, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(3, 3+i, BRIGHTNESS);
    }
    delay(200);

  // Connect MCP2
  if(!mcp2.begin_I2C(0x21,&Wire2))
  {
    Serial.println("Error with MCP2");
  } else Serial.println("MCP2 Up!");
  
    // Loading bar
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(2, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(1, 3+i, BRIGHTNESS);
    }
    delay(200);

  // Status update
  Serial.println("I2C Ready");
  
    // Loading bar
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(0, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(8, 3+i, BRIGHTNESS);
    }
    delay(200);

  // Status update
  Serial.println("Setting up button pins");

  // Setup button pins
  pinMode(OCT_DOWN_PIN, INPUT_PULLUP);
  pinMode(OCT_UP_PIN, INPUT_PULLUP);
  pinMode(SHIFT_PIN, INPUT_PULLUP);
  pinMode(HOLD_PIN, INPUT_PULLUP);
  pinMode(ARP_PIN, INPUT_PULLUP);

    // Loading bar
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(9, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(10, 3+i, BRIGHTNESS);
    }
    delay(200);

  // Status update  
  Serial.println("Setting up button interrupts");

  // Attach interrupts to buttons
  attachInterrupt(digitalPinToInterrupt(OCT_DOWN_PIN), octaveDownInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(OCT_UP_PIN), octaveUpInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(SHIFT_PIN), shiftInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HOLD_PIN), holdInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ARP_PIN), arpInterrupt, CHANGE);
  
    // Loading bar
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(11, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(12, 3+i, BRIGHTNESS);
    }
    delay(200);

  // Status update
  Serial.println("Starting TRS communication");

  // Connect TRS output
  TRS.begin();
  
    // Loading bar
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(13, 3+i, BRIGHTNESS);
    }
    delay(200);
    for(uint8_t i = 0; i < 3; i++)
    {
      led_Matrix.drawPixel(14, 3+i, BRIGHTNESS);
    }
    delay(200);
    led_Matrix.clear();

  // Status update
  Serial.println("Setting LEDs");

  //Initialize LEDs
  led_Matrix.drawPixel(OCT_LEDS_X[2], OCT_LEDS_Y[2], BRIGHTNESS);
  led_Matrix.drawPixel(SET_NUM_LEDS_X[3], SET_NUM_LEDS_Y[3], BRIGHTNESS);
  led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[0], ARP_PATTERN_LEDS_Y[0], BRIGHTNESS);
  led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[0], NOTE_LENGTH_LEDS_Y[0], BRIGHTNESS);

  // Status update
  Serial.println("Starting loop.");
  
  // Start arpeggio timer
  arp_Timer.begin(arpHandler, (float)BPM_CONV/float(_tempo * arp_Buffer.note_length));
}
/*********************************************************************************/

/*********************************** MAIN LOOP ***********************************/
void loop() {
  static long current_micros = 0;
  static long key_exit_micros = 0;

  current_micros = micros();
  // Check keys and adjust LEDs every 5ms
  if(current_micros - key_exit_micros > 5000)
  {
    checkKeys();
    if(_adjust_key_LEDs) adjustKeyLEDs();
    if(_turn_notes_off) turnNotesOff();
    key_exit_micros = micros();
  }
}
/*********************************************************************************/

/**************************** KEY HANDLING FUNCTIONS *****************************/

// Check key states and send MIDI commands based on mode/feature selections
void checkKeys()
{
  // Variables to store two states and the difference between them
  static uint16_t mcp1_current_state = 0;
  static uint16_t mcp1_previous_state = 0;
  static uint16_t mcp1_change_in_state = 0;

  static uint16_t mcp2_current_state = 0;
  static uint16_t mcp2_previous_state = 0;
  static uint16_t mcp2_change_in_state = 0;

  // Read current states from GPIO expanders
  mcp1_current_state = mcp1.readGPIOAB();
  mcp2_current_state = mcp2.readGPIOAB();
  
  // Disregard communication hiccups
  if(mcp1_current_state == 65535)
  {
    mcp1_change_in_state = 0;
    Serial.println("Splat condition on MCP1");
  }
  // If read states make sense, find what changed from previous state
  else
  {
    mcp1_change_in_state = mcp1_current_state ^ mcp1_previous_state;
  }

  // If anything changed
  if(mcp1_change_in_state)
  {
    // Create a local copy of the feature settings
    uint8_t current_features = _feature_select;

    // Loop through the relevant bits
    for(uint8_t i = 0; i < MCP1_PIN_COUNT; i++)
    {
      // Bit mask shifts left every iteration
      uint16_t bitMask = 1 << i;
      
      // Was there a change?
      if(mcp1_change_in_state & bitMask)
      {
        // Calculate the note value, first GPIO expander starts at C3
        uint8_t note = C3 + i + _octave_num * OCTAVE;
        uint8_t velocity = 99;

        // For key press
        if(mcp1_current_state & bitMask)
        {
          // Change key press behaviour based on feature selection
          switch(current_features)
          {
            case ARP_HOLD:
              if(!arpBufferRemove(&arp_Buffer, note))
              {
                if(!arpBufferAdd(&arp_Buffer, note, velocity))
                {
                  startNote(note, velocity);
                }
              }
              break;

            case ARP:
              if(!arpBufferAdd(&arp_Buffer, note, velocity))
              {
                startNote(note, velocity);
              }
              break;
            
            case HOLD:
              if(_hold_mode == HOLD_ALL)
              {
                if(_active_notes[(note - C1) / OCTAVE] & (1 << (note % OCTAVE)))
                {
                  stopNote(note);
                }
                else
                {
                  startNote(note, velocity);
                }
              }
              else
              {
                if(!holdBufferRemove(&hold_Buffer, note))
                {
                  if(!holdBufferAdd(&hold_Buffer, note))
                  {
                    startNote(note, velocity);
                  }
                  else
                  {
                    startNote(note, velocity);
                  }
                }
              }
              break;

            case STANDARD:
              startNote(note, velocity);
              break;

            default:
              break;
          }
        }
        // For key release
        else
        {
          // Change key release feature based on feature selection
          switch(current_features)
          {
            case ARP_HOLD:
              if(!arpBufferIncludes(&arp_Buffer, note))
              {
                stopNote(note);
              }
              break;

            case ARP:
              if(!arpBufferRemove(&arp_Buffer, note))
              {
                stopNote(note);
              }
              break;
            
            case HOLD:
              if(_hold_mode == HOLD_SET)
              {
                if(!holdBufferIncludes(&hold_Buffer, note))
                {
                  stopNote(note);
                }
              }
              break;

            case STANDARD:
              stopNote(note);
              break;

            default:
              break;

          }
        }
        // Set previous_state to current_state for next check
        mcp1_previous_state = mcp1_current_state;
      }
    }
  }
  
  // Repeat steps for second GPIO expander
  if(mcp2_current_state == 65535)
  {
    mcp2_change_in_state = 0;
    Serial.println("Splat condition on MCP2");
  }
  else
  {
    mcp2_change_in_state = mcp2_current_state ^ mcp2_previous_state;
  }
  if(mcp2_change_in_state)
  {
    uint8_t current_features = _feature_select;
    for(uint8_t i = 0; i < MCP2_PIN_COUNT; i++)
    {
      uint16_t bitMask = 1 << i;
      if(mcp2_change_in_state & bitMask)
      {
        // Second GPIO expander starts at C4
        uint8_t note = C4 + i + _octave_num * OCTAVE;
        uint8_t velocity = 99;
        if(mcp2_current_state & bitMask)
        {
          switch(current_features)
          {
            case ARP_HOLD:
              if(!arpBufferRemove(&arp_Buffer, note))
              {
                if(!arpBufferAdd(&arp_Buffer, note, velocity))
                {
                  startNote(note, velocity);
                }
              }
              break;

            case ARP:
              if(!arpBufferAdd(&arp_Buffer, note, velocity))
              {
                startNote(note, velocity);
              }
              break;
            
            case HOLD:
              if(_hold_mode == HOLD_ALL)
              {
                if(_active_notes[(note - C1) / OCTAVE] & (1 << (note % OCTAVE)))
                {
                  stopNote(note);
                }
                else
                {
                  startNote(note, velocity);
                }
              }
              else
              {
                if(!holdBufferRemove(&hold_Buffer, note))
                {
                  if(!holdBufferAdd(&hold_Buffer, note))
                  {
                    startNote(note, velocity);
                  }
                  else
                  {
                    startNote(note, velocity);
                  }
                }
              }
              break;

            case STANDARD:
              startNote(note, velocity);
              break;

            default:
              break;
          }
        }
        else
        {
          switch(current_features)
          {
            case ARP_HOLD:
              if(!arpBufferIncludes(&arp_Buffer, note))
              {
                stopNote(note);
              }
              break;

            case ARP:
              if(!arpBufferRemove(&arp_Buffer, note))
              {
                stopNote(note);
              }
              break;
            
            case HOLD:
              if(_hold_mode == HOLD_SET)
              {
                if(!holdBufferIncludes(&hold_Buffer, note))
                {
                  stopNote(note);
                }
              }
              break;

            case STANDARD:
              stopNote(note);
              break;

            default:
              break;

          }
        }
      }
      mcp2_previous_state = mcp2_current_state;
    }
  }
}

// Sends a note on command, adjusts the note flag, and signals that LEDs need to be adjusted
void startNote(uint8_t note, uint8_t velocity)
{
  usbMIDI.sendNoteOn(note, velocity, USB_CHANNEL);
  TRS.sendNoteOn(note, velocity, TRS_CHANNEL);
  if(!(_active_notes[(note - C1) / OCTAVE] & (1 << (note % OCTAVE))))
  {
     _active_notes[(note - C1) / OCTAVE] += (1 << (note % OCTAVE));
  }
  _adjust_key_LEDs = 1;
}

// Sends a note off command, adjusts the note flag, and signals that LEDs need to be adjusted
void stopNote(uint8_t note)
{
  usbMIDI.sendNoteOff(note, 0, USB_CHANNEL);
  TRS.sendNoteOn(note, 0, TRS_CHANNEL);
  if(_active_notes[(note - C1) / OCTAVE] & (1 << (note % OCTAVE)))
  {
     _active_notes[(note - C1) / OCTAVE] -= (1 << (note % OCTAVE));
  }
  _adjust_key_LEDs = 1;
}

// Set LEDs for keybed
void adjustKeyLEDs()
{
  for(uint8_t i = 0; i < 13; i++)
  {
    uint16_t bit_mask = 1 << (i % OCTAVE);
    // Left side
    if(_active_notes[2 + _octave_num + i / OCTAVE] & bit_mask)
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[i], TOUCHPAD_LEDS_Y[i], BRIGHTNESS);
    }
    else
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[i], TOUCHPAD_LEDS_Y[i], 0);
    }
    // Right side
    if(_active_notes[3 + _octave_num + i / OCTAVE] & bit_mask)
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[12 + i], TOUCHPAD_LEDS_Y[12 + i], BRIGHTNESS);
    }
    else
    {
      led_Matrix.drawPixel(TOUCHPAD_LEDS_X[12 + i], TOUCHPAD_LEDS_Y[12 + i], 0);
    }
  }
  led_Matrix.drawPixel(DIFFICULT_LED_X, DIFFICULT_LED_Y, 0);
  _adjust_key_LEDs = 0;
}

// Loop through _active_notes[] to turn off any sustained notes
void turnNotesOff()
{
  for(uint8_t i = 0; i < 8; i++)
  {
    {
      if( _active_notes[i])
      {
        for(int j = 0; j < OCTAVE; j++)
        {
          u_int16_t bit_mask = 1 << j;
          if(bit_mask &  _active_notes[i])
          {
            usbMIDI.sendNoteOff(C2 + OCTAVE*(i-1) + j, 0, USB_CHANNEL);
            TRS.sendNoteOff(C2 + OCTAVE*(i-1) + j, 0, TRS_CHANNEL);
            led_Matrix.drawPixel(TOUCHPAD_LEDS_X[OCTAVE * (i-1) + j - OCTAVE], TOUCHPAD_LEDS_Y[OCTAVE * (i-1) + j - OCTAVE], 0);
             _active_notes[i] -= bit_mask;
          }
        }
        _adjust_key_LEDs = 1;
      }
    }
  }
  _turn_notes_off = 0;
}
/*********************************************************************************/

/*************************** BUTTON HANDLING FUNCTIONS ***************************/
// Octave Up button handling
void octaveUpInterrupt()
{
  // Debounce
  static long lastInterruptMillis = 0;
  unsigned long currentMillis = millis();

  // Button debounce
  if(currentMillis - lastInterruptMillis > DEBOUNCE_MS)
  {
    // Basic behaviour
    if(!_shift_on)
    {
      if(_octave_num < 3)
      {
        led_Matrix.drawPixel(OCT_LEDS_X[_octave_num+2], OCT_LEDS_Y[_octave_num+2], 0);
        _octave_num++;
        led_Matrix.drawPixel(OCT_LEDS_X[_octave_num+2], OCT_LEDS_Y[_octave_num+2], BRIGHTNESS);
        _adjust_key_LEDs = 1;
      }
    }
    else
    {
      uint8_t current_features = _feature_select;

      // SHIFT behaviours
      switch(current_features)
      {
        case ARP:
          arpBufferCyclePattern(&arp_Buffer);
          break;
        
        case ARP_HOLD:
          setSelectUp();
          holdBufferInit(&hold_Buffer);
          arpBufferReInit(&arp_Buffer);
          break;

        case HOLD:
          if(_hold_mode == HOLD_SET)
          {
            _hold_mode -= HOLD_SET;
            holdBufferInit(&hold_Buffer);
            led_Matrix.drawPixel(HOLD_SET_LED_X, HOLD_SET_LED_Y, 0);
            led_Matrix.drawPixel(HOLD_ALL_LED_X, HOLD_ALL_LED_Y, BRIGHTNESS);
          }
          break;

        case STANDARD:
          _tempo += 5;
          break;

        default:
          break;
      }
    }
    lastInterruptMillis = millis();
  }
}

// Octave Down button handling
void octaveDownInterrupt()
{
  static long lastInterruptMillis = 0;
  unsigned long currentMillis = millis();

  // Debounce button
  if(currentMillis - lastInterruptMillis > DEBOUNCE_MS)
  {
    // Basic behaviour
    if(!_shift_on)
    {
      if(_octave_num > -2)
      {
        led_Matrix.drawPixel(OCT_LEDS_X[_octave_num+2], OCT_LEDS_Y[_octave_num+2], 0);
        _octave_num--;
        led_Matrix.drawPixel(OCT_LEDS_X[_octave_num+2], OCT_LEDS_Y[_octave_num+2], BRIGHTNESS);
        _adjust_key_LEDs = 1;
      }
    }
    else
    {
      uint8_t current_features = _feature_select;

      // SHIFT behaviours
      switch(current_features)
      {
        case ARP:
          arpBufferNoteLength(&arp_Buffer);
          break;
        
        case ARP_HOLD:
          setSelectDown();
          holdBufferInit(&hold_Buffer);
          arpBufferReInit(&arp_Buffer);
          break;

        case HOLD:
          if(_hold_mode == HOLD_ALL)
          {
            _hold_mode += HOLD_SET;
            led_Matrix.drawPixel(HOLD_ALL_LED_X, HOLD_ALL_LED_Y, 0);
            led_Matrix.drawPixel(HOLD_SET_LED_X, HOLD_SET_LED_Y, BRIGHTNESS);
          }
          break;

        case STANDARD:
          _tempo += 5;
          break;

        default:
          break;
      }
    }
    lastInterruptMillis = millis();
  }
}

// HOLD button handling
void holdInterrupt()
{
  static long lastInterruptMillis = 0;
  unsigned long currentMillis = millis();

  if(currentMillis - lastInterruptMillis > DEBOUNCE_MS)
  {
    uint8_t current_settings = _feature_select;

    // HOLD button behaviour based on current feature selections
    switch(current_settings)
    {
      case ARP_HOLD:
        _feature_select = current_settings - HOLD;
        holdBufferInit(&hold_Buffer);
        if(_hold_mode == HOLD_SET) led_Matrix.drawPixel(HOLD_SET_LED_X, HOLD_SET_LED_Y, 0);
        else led_Matrix.drawPixel(HOLD_ALL_LED_X, HOLD_ALL_LED_Y, 0);
        _turn_notes_off = 1;
        break;

      case ARP:
        _feature_select = current_settings + HOLD;
        if(_hold_mode == HOLD_SET) led_Matrix.drawPixel(HOLD_SET_LED_X, HOLD_SET_LED_Y, BRIGHTNESS);
        else led_Matrix.drawPixel(HOLD_ALL_LED_X, HOLD_ALL_LED_Y, BRIGHTNESS);
        break;

      case HOLD:
        _feature_select = current_settings - HOLD;
        holdBufferInit(&hold_Buffer);
        if(_hold_mode == HOLD_SET) led_Matrix.drawPixel(HOLD_SET_LED_X, HOLD_SET_LED_Y, 0);
        else led_Matrix.drawPixel(HOLD_ALL_LED_X, HOLD_ALL_LED_Y, 0);
        _turn_notes_off = 1;
        break;

      case STANDARD:
        _feature_select = current_settings + HOLD;
        if(_hold_mode == HOLD_SET) led_Matrix.drawPixel(HOLD_SET_LED_X, HOLD_SET_LED_Y, BRIGHTNESS);
        else led_Matrix.drawPixel(HOLD_ALL_LED_X, HOLD_ALL_LED_Y, BRIGHTNESS);
        break;

      default:
        break;
    }
  }
  lastInterruptMillis = millis();
}

// ARP button handling
void arpInterrupt()
{
  static long lastInterruptMillis = 0;
  unsigned long currentMillis = millis();

  if(currentMillis - lastInterruptMillis > DEBOUNCE_MS)
  {
    uint8_t current_settings = _feature_select;

    // ARP behaviour based on current feature selections
    switch(current_settings)
    {
      case ARP_HOLD:
        _feature_select = current_settings - ARP;
        _turn_notes_off = 1;
        led_Matrix.drawPixel(ARP_LED_X, ARP_LED_Y, 0);
        arpBufferReInit(&arp_Buffer);
        break;

      case ARP:
        _feature_select = current_settings - ARP;
        _turn_notes_off = 1;
        led_Matrix.drawPixel(ARP_LED_X, ARP_LED_Y, 0);
        arpBufferReInit(&arp_Buffer);
        break;

      case HOLD:
        _feature_select = current_settings + ARP;
        led_Matrix.drawPixel(ARP_LED_X, ARP_LED_Y, BRIGHTNESS);
        break;

      case STANDARD:
        _feature_select = current_settings + ARP;
        led_Matrix.drawPixel(ARP_LED_X, ARP_LED_Y, BRIGHTNESS);
        break;

      default:
        break;
    }
  }
  lastInterruptMillis = millis();
}

// SHIFT button handling
void shiftInterrupt()
{
  static long lastInterruptMillis = 0;
  unsigned long currentMillis = millis();

  if(currentMillis - lastInterruptMillis > DEBOUNCE_MS)
  {
    if(!_shift_on)
    {
      _shift_on = 1;
      led_Matrix.drawPixel(SHIFT_LED_X, SHIFT_LED_Y, BRIGHTNESS);
    }
    else
    {
      _shift_on = 0;
      led_Matrix.drawPixel(SHIFT_LED_X, SHIFT_LED_Y, 0);
    }
  }
  lastInterruptMillis = millis();
}
/*********************************************************************************/

/**************************** SET SELECTION FUNCTIONS ****************************/
// Helper function to increase SET selection
void setSelectUp()
{
  uint8_t current_set = _set_select;
  switch(current_set)
  {
    case 1:
      current_set = 2;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[0], SET_NUM_LEDS_Y[0], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[1], SET_NUM_LEDS_Y[1], BRIGHTNESS);
      break;

    case 2:
      current_set = 3;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[1], SET_NUM_LEDS_Y[1], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[2], SET_NUM_LEDS_Y[2], BRIGHTNESS);
      break;

    case 3:
      current_set = 4;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[2], SET_NUM_LEDS_Y[2], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[3], SET_NUM_LEDS_Y[3], BRIGHTNESS);
      break;

    case 4:
      current_set = 6;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[3], SET_NUM_LEDS_Y[3], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[4], SET_NUM_LEDS_Y[4], BRIGHTNESS);
      break;

    case 6:
      current_set = 8;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[4], SET_NUM_LEDS_Y[4], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[5], SET_NUM_LEDS_Y[5], BRIGHTNESS);
      break;

    default:
      break;  
  }
  _set_select = current_set;
}

// Helper function to decrease SET selection
void setSelectDown()
{
  uint8_t current_set = _set_select;
  switch(current_set)
  {
    case 8:
      current_set = 6;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[5], SET_NUM_LEDS_Y[5], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[4], SET_NUM_LEDS_Y[4], BRIGHTNESS);
      break;

    case 6:
      current_set = 4;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[4], SET_NUM_LEDS_Y[4], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[3], SET_NUM_LEDS_Y[3], BRIGHTNESS);
      break;

    case 4:
      current_set = 3;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[3], SET_NUM_LEDS_Y[3], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[2], SET_NUM_LEDS_Y[2], BRIGHTNESS);
      break;

    case 3:
      current_set = 2;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[2], SET_NUM_LEDS_Y[2], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[1], SET_NUM_LEDS_Y[1], BRIGHTNESS);
      break;

    case 2:
      current_set = 1;
      led_Matrix.drawPixel(SET_NUM_LEDS_X[1], SET_NUM_LEDS_Y[1], 0);
      led_Matrix.drawPixel(SET_NUM_LEDS_X[0], SET_NUM_LEDS_Y[0], BRIGHTNESS);
      break;

    default:
      break;  
  }
  _set_select = current_set;
}
/*********************************************************************************/

/***************************** ARP BUFFER FUNCTIONS ******************************/
// TEMPO and arpeggio handler
void arpHandler()
{
  static uint8_t arp_note_playing = 0;
  static uint8_t arp_current_note = 0;
  static uint8_t arp_position = 0;
  static uint8_t interval_timer_passes = 0;

  Arp_Buffer* buffer = &arp_Buffer;

  if(_arp_reset)
  {
    arp_note_playing = 0;
    arp_current_note = 0;
    arp_position = 0;
    interval_timer_passes = 0;
    _arp_reset = 0;
  }
 
  // Control TEMPO_LED
  if(interval_timer_passes < (2 * buffer->note_length))
  {
    led_Matrix.drawPixel(TEMPO_LED_X, TEMPO_LED_Y, BRIGHTNESS);
  }
  else
  {
    led_Matrix.drawPixel(TEMPO_LED_X, TEMPO_LED_Y, 0);
  }

  // Control ARP
  if(_feature_select & ARP)
  {    
    if((interval_timer_passes % ARP_TIME_DIVIDER) == 0)
    {
      uint8_t note;
      uint8_t velocity;
      if(arpBufferGet(&arp_Buffer, arp_position, &note, &velocity))
      {
        arp_current_note = note;
        startNote(note, velocity);
        arp_note_playing = 1;

        arp_position++;

        uint8_t cycle_length;
        if(buffer->pattern == ARP_UP_DOWN && buffer->note_count > 1)
        {
          cycle_length = buffer->note_count * 2 - 2;
        }
        else
        {
          cycle_length = buffer->note_count;
        }
        if(arp_position >= cycle_length)
        {
          arp_position = 0;
        }
      }
    }

    if(arp_note_playing && ((interval_timer_passes % ARP_TIME_DIVIDER) == 1))
    {
      stopNote(arp_current_note);
      arp_note_playing = 0;
    }
  }
  interval_timer_passes = (interval_timer_passes + 1) % (4 * buffer->note_length);
}

// Arp Buffer initializer
void arpBufferInit(Arp_Buffer* buffer)
{
  buffer->write_index = 0;
  buffer->read_index = 0;
  buffer->note_count = 0;
  buffer->pattern = ARP_UP;
  buffer->set_select = _set_select;
  buffer->note_length = QUARTER;
}

// Arp Buffer reinitializer
void arpBufferReInit(Arp_Buffer* buffer)
{
  buffer->write_index = 0;
  buffer->read_index = 0;
  buffer->note_count = 0;
  buffer->set_select = _set_select;
}

// Helper function to adjust ARP Note Length selection
void arpBufferNoteLength(Arp_Buffer* buffer)
{
  uint8_t current_note_length = buffer->note_length;
  switch(current_note_length)
  {
    case QUARTER:
      led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[0], NOTE_LENGTH_LEDS_Y[0], 0);
      buffer->note_length = EIGHTH;
      led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[1], NOTE_LENGTH_LEDS_Y[1], BRIGHTNESS);
      break;

    case EIGHTH:
      led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[1], NOTE_LENGTH_LEDS_Y[1], 0);
      buffer->note_length = SIXTEENTH;
      led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[2], NOTE_LENGTH_LEDS_Y[2], BRIGHTNESS);
      break;

    case SIXTEENTH:
      led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[2], NOTE_LENGTH_LEDS_Y[2], 0);
      buffer->note_length = QUARTER;
      led_Matrix.drawPixel(NOTE_LENGTH_LEDS_X[0], NOTE_LENGTH_LEDS_Y[0], BRIGHTNESS);
      break;

    default:
      break;
  }
  arp_Timer.update((float)BPM_CONV/float(_tempo * arp_Buffer.note_length));
}

// Helper function to cycle ARP Mode/Pattern
void arpBufferCyclePattern(Arp_Buffer* buffer)
{
  cli();
  switch(buffer->pattern)
  {
    case ARP_UP:
      buffer->pattern = ARP_DOWN;
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[0], ARP_PATTERN_LEDS_Y[0], 0);
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[1], ARP_PATTERN_LEDS_Y[1], BRIGHTNESS);
      break;

    case ARP_DOWN:
      buffer->pattern = ARP_UP_DOWN;
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[1], ARP_PATTERN_LEDS_Y[1], 0);
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[2], ARP_PATTERN_LEDS_Y[2], BRIGHTNESS);
      break;

    case ARP_UP_DOWN:
      buffer->pattern = ARP_CUSTOM;
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[2], ARP_PATTERN_LEDS_Y[2], 0);
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[3], ARP_PATTERN_LEDS_Y[3], BRIGHTNESS);
      break;

    case ARP_CUSTOM:
      buffer->pattern = ARP_UP;
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[3], ARP_PATTERN_LEDS_Y[3], 0);
      led_Matrix.drawPixel(ARP_PATTERN_LEDS_X[0], ARP_PATTERN_LEDS_Y[0], BRIGHTNESS);
      break;

    default:
      buffer->pattern = ARP_UP;
      break;
  }
  sei();
}

// Helper function to sort elements in the Arp Buffer
void arpBufferSort(Arp_Buffer* buffer)
{
  // Skip if empty or only one element in buffer
  if(buffer->note_count < 2) return;

  // Temporary arrays to sort buffer notes and velocities
  uint8_t sortedNotes[MAX_SET_NOTES];
  uint8_t sortedVelocities[MAX_SET_NOTES];

  // Start at the current read_index (necessary?)
  uint8_t index = buffer->read_index;

  // Copy the current buffer notes and velocities to the temp arrays
  for(uint8_t i = 0; i < buffer->note_count; i++)
  {
    sortedNotes[i] = buffer->notes[index];
    sortedVelocities[i] = buffer->velocities[index];
    index = (index + 1) % MAX_SET_NOTES;
  }

  // Insertion sort!
  for(uint8_t i = 1; i < buffer->note_count; i++)
  {
    uint8_t keyNote = sortedNotes[i];
    uint8_t keyVelocity = sortedVelocities[i];
    int j = i - 1;

    while(j >= 0 && sortedNotes[j] > keyNote)
    {
      sortedNotes[j+1] = sortedNotes[j];
      sortedVelocities[j+1] = sortedVelocities[j];
      j--;
    }

    sortedNotes[j+1] = keyNote;
    sortedVelocities[j+1] = keyVelocity;
  }

  // Copy the arrays back into the buffer
  index = buffer->read_index;
  for(uint8_t i = 0; i < buffer->note_count; i++)
  {
    buffer->notes[index] = sortedNotes[i];
    buffer->velocities[index] = sortedVelocities[i];
    index = (index + 1) % MAX_SET_NOTES;
  }
}

// Helper function to add notes to the Arp Buffer
uint8_t arpBufferAdd(Arp_Buffer* buffer, uint8_t note, uint8_t velocity)
{
  // Check for out of bounds
  if(buffer->note_count >= buffer->set_select)
  {
    return 0;
  }
  else
  {
    // Write the new note into the buffer
    buffer->notes[buffer->write_index] = note;
    buffer->velocities[buffer->write_index] = velocity;

    // Increment write_index and note_count
    buffer->write_index = (buffer->write_index + 1) % MAX_SET_NOTES;
    buffer->note_count++;

    // Sort the buffer
    if(!(buffer->pattern == ARP_CUSTOM))
    {
      arpBufferSort(buffer);
    }
    
    if(buffer->note_count == 1)
    {
      arp_Timer.end();
      _arp_reset = 1;
      arp_Timer.begin(arpHandler, (float)BPM_CONV/(float)(_tempo * buffer->note_length));
    }

    return 1;
  }
}

// Helper function to remove notes from the Arp Buffer
uint8_t arpBufferRemove(Arp_Buffer* buffer, uint8_t note)
{
  if(buffer->note_count == 0) 
  {
    return 0;
  }

  uint8_t index = buffer->read_index;

  // Search for the note in the buffer
  for(uint8_t i = 0; i < buffer->note_count; i++)
  {
    // Once found, remove it and shift the rest of the elements to fill the gap
    if(buffer->notes[index] == note)
    {
      for(uint8_t j = i; j < (buffer->note_count - 1); j++)
      {
        uint8_t current = (buffer->read_index + j) % MAX_SET_NOTES;
        uint8_t next = (buffer->read_index + j + 1) % MAX_SET_NOTES;
        buffer->notes[current] = buffer->notes[next];
        buffer->velocities[current] = buffer->velocities[next];
      }
      // Decrement the write_index and note_count
      buffer->write_index = (buffer->write_index - 1) % MAX_SET_NOTES;
      buffer->note_count--;

      // Return 1 and leave the function
      return 1;
    }
    // Increment the index, resetting it if hitting the end of the buffer
    // Probably need to change for cycle_length
    index = (index + 1) % MAX_SET_NOTES;
    
  }

  // Return 0 if note isn't found
  return 0;
}

// Helper function to check if Arp Buffer contains a specified note
uint8_t arpBufferIncludes(Arp_Buffer* buffer, uint8_t note)
{
  if(buffer->note_count == 0)
  {
    return 0;
  }

  for(uint8_t i = 0; i < buffer->note_count; i++)
  {
    if(buffer->notes[i] == note)
    {
      return 1;
    }
  }

  return 0;
}

// Helper function to read notes from the Arp Buffer
uint8_t arpBufferGet(Arp_Buffer* buffer, uint8_t position, uint8_t* note, uint8_t* velocity)
{
  // Return 0 if buffer is empty
  if(buffer->note_count == 0)
  {
    return 0;
  }

  // Adjust actual position in full cycle based on buffer->pattern
  uint8_t actual_position;
  switch(buffer->pattern)
  {
    // Increase until overflow, then start at the beginning
    case ARP_UP:
      actual_position = position % buffer->note_count;
      break;

    // Start at the end, adjust the subtraction by referenced position
    case ARP_DOWN:
      actual_position = (buffer->note_count - 1) - (position % buffer->note_count);
      break;

    // Use cycle_length to create an imaginary extended buffer including
    // ascending and decending notes, i.e. C, E, G, C, G, E, G, C
    case ARP_UP_DOWN:
      {
        if(buffer->note_count == 1)
        {
          actual_position = 0;
        }
        else
        {
          uint8_t cycle_length = buffer->note_count * 2 - 2;
          uint8_t cycle_position = position % cycle_length;

          if(cycle_position < buffer->note_count)
          {
            actual_position = cycle_position;
          }
          else
          {
            actual_position = buffer->note_count - (cycle_position - buffer->note_count) - 2;
          }
        }
      }
      break;

    default:
      actual_position = position % buffer->note_count;
      break;
  }
  
  // Fill the variables pointed to based on the selected index
  uint8_t index = (buffer->read_index + actual_position) % _set_select;
  *note = buffer->notes[index];
  *velocity = buffer->velocities[index];

  // Return 1 for successful buffer get
  return 1;
}
/*********************************************************************************/

/***************************** HOLD BUFFER FUNCTIONS *****************************/
// Hold Buffer initializer
void holdBufferInit(Hold_Buffer* buffer)
{
  buffer->note_count = 0;
  buffer->set_select = _set_select;
}

// Helper function to remove a note from the Hold Buffer
uint8_t holdBufferRemove(Hold_Buffer* buffer, uint8_t note)
{
  // If buffer is empty, return false
  if(buffer->note_count == 0)
  {
    return 0;
  }

  // Loop through note array to find note, return true if successful
  for(uint8_t i = 0; i < buffer->note_count; i++)
  {
    if(buffer->notes[i] == note)
    {
      for(uint8_t j = i; j < (buffer->note_count - 1); j++)
      {
        buffer->notes[j] = buffer->notes[j + 1];
      }
      buffer->note_count--;

      return 1;
    }
  }
  
  // Return false if note not found
  return 0;
}

// Helper function to check if the Hold Buffer contains a note
uint8_t holdBufferIncludes(Hold_Buffer* buffer, uint8_t note)
{
  if(buffer->note_count == 0)
  {
    return 0;
  }

  for(uint8_t i = 0; i < buffer->note_count; i++)
  {
    if(buffer->notes[i] == note)
    {
      return 1;
    }
  }

  return 0;
}

// Helper function to add a note to the Hold Buffer
uint8_t holdBufferAdd(Hold_Buffer* buffer, uint8_t note)
{
  // If buffer is filled up to selected set number, return false
  if(buffer->note_count >= buffer->set_select)
  {
    return 0;
  }

  // Add the note to the buffer, increment the note_count, and return true
  buffer->notes[buffer->note_count] = note;
  buffer->note_count++;

  return 1;
}
/*********************************************************************************/
/******************************* END PROGRAM *************************************/
/*********************************************************************************/