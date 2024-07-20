// Inspired by this instructable by user anouskadg: https://www.instructables.com/MIDI-Arduino-Drums/
// Originally adapted from Jenna deBoisblanc and Spiekenzie Labs initial code
// https://www.instructables.com/Arduino-Xylophone/

// Adjustments made to original code:
// - Mapping to Ugritone Kult Drums II notes (https://ugritone.com/products/kvlt-drums-ii)
// - Added a pushbutton to change pin to note mapping
// - Added a pushbutton to change output from MIDI channel 1 to 16
// - Added 4 LEDs to show currently selected MIDI channel in binary
// - Added digital inputs for additional button-based triggers
// - Added LED visual effect

//*******************************************************************************************************************
// User settable variables
//*******************************************************************************************************************
const int numAnalogInputs = 15; // Number of analog inputs
const int numDigitalInputs = 3; // Number of digital inputs

const int analogPinAssignments[numAnalogInputs] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14}; // Analog pins
const int digitalPinAssignments[numDigitalInputs] = {2, 3, 4}; // Digital pins for additional inputs

// LED PWM pins
//const int ledPins[3] = {9, 10, 11}; // PWM pins for LEDs

// Ugritone Kult Drums II mapping
const byte PadNote[numAnalogInputs + numDigitalInputs] = {
  32, // A0 // Hihat open
  42, // A1 // Hihat closed
  44, // A2 // Hihat pedal
  38, // A3 // Snare
  41, // A4 // Tom5
  51, // A5 // Ride top
  36, // A6 // Kick
  36, // A7 // Kick
  48, // A8 // Tom1
  47, // A9 // Tom2
  45, // A10 // Tom3
  43, // A11 // Tom4
  52, // A12 // China1
  57, // A13 // Crash2
  59, // A14 // Fxcymbal2
  49, // D2 // Crash1
  61, // D3 // China2
  56  // D4 // Fxcymbal1
};

const int PadCutOff[numAnalogInputs] = {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200}; // Minimum Analog value to cause a drum hit
const int MaxPlayTime[numAnalogInputs + numDigitalInputs] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60}; // Cycles before a 2nd hit is allowed

int midichannel = 0; // MIDI channel from 0 to 15 (+1 in "real world")
const boolean VelocityFlag = true; // Velocity ON (true) or OFF (false)

// NOTE BANK
const byte NoteBank[23] = {
  32, // Hihat wideopen
  36, // Kick
  38, // Snare
  41, // Tom5
  42, // HiHat closed
  43, // Tom4
  44, // HiHat pedal
  45, // Tom3
  46, // HiHat open
  47, // Tom2
  48, // Tom1
  49, // Crash1
  50, // Crash1 quick
  51, // Ride top
  52, // China1
  53, // Ride bell
  54, // Ride crash
  55, // Splash1
  56, // FxCymbal1
  57, // Crash2
  58, // Crash2 quick
  59, // FxCymbal2
  61  // China2
};

//*******************************************************************************************************************
// Internal Use Variables
//*******************************************************************************************************************
boolean activePad[numAnalogInputs + numDigitalInputs] = {0};
int PinPlayTime[numAnalogInputs + numDigitalInputs] = {0};

byte status1;
int hitavg = 0;

//unsigned long lastLedUpdate = 0; // Timestamp of the last LED update
//int ledBrightness[3] = {0}; // Brightness levels of LEDs
//int ledFadeDirection[3] = {1, 1, 1}; // Fade direction of LEDs: 1 for increasing, -1 for decreasing

/**
  Setup
*/
void setup() {
  Serial.begin(115200); // connect to the serial port 115200
  
  // Set up digital pins for input
  for (int i = 0; i < numDigitalInputs; i++) {
    pinMode(digitalPinAssignments[i], INPUT_PULLUP);
  }

//   // Set up LED pins for output (add current-limiting resistors to each LED)
//   for (int i = 0; i < 3; i++) {
//     pinMode(ledPins[i], OUTPUT);
//     analogWrite(ledPins[i], 0); // Initialize LEDs to off
//   }

 }

/**
  Read input pads and send MIDI signals
*/
void readMidi() {
  // Read analog inputs
  for (int pin = 0; pin < numAnalogInputs; pin++) {
    hitavg = analogRead(analogPinAssignments[pin]);

    if (hitavg > PadCutOff[pin]) {
      if (!activePad[pin]) {
        if (VelocityFlag) {
          hitavg = (hitavg / 8) - 1; // Upper range
        } else {
          hitavg = 127;
        }

        MIDI_TX(144, PadNote[pin], hitavg);
        PinPlayTime[pin] = 0;
        activePad[pin] = true;
      } else {
        PinPlayTime[pin]++;
      }
    } else if (activePad[pin]) {
      PinPlayTime[pin]++;
      if (PinPlayTime[pin] > MaxPlayTime[pin]) {
        activePad[pin] = false;
        MIDI_TX(144, PadNote[pin], 0);
      }
    }
  }
  
  // Read digital inputs
  for (int i = 0; i < numDigitalInputs; i++) {
    int state = digitalRead(digitalPinAssignments[i]);
    int pinIndex = numAnalogInputs + i; // digital inputs are mapped after analog inputs in PadNote array

    if (state == LOW && !activePad[pinIndex]) { // button pressed
      MIDI_TX(144, PadNote[pinIndex], 100); // Set a more reasonable fixed velocity for digital inputs
      PinPlayTime[pinIndex] = 0;
      activePad[pinIndex] = true;
    } else if (state == HIGH && activePad[pinIndex]) { // button released
      PinPlayTime[pinIndex]++;
      if (PinPlayTime[pinIndex] > MaxPlayTime[pinIndex]) {
        activePad[pinIndex] = false;
        MIDI_TX(144, PadNote[pinIndex], 0);
      }
    }
  }
}

/**
  Update LEDs for a cool visual effect
*/
// void updateLEDs() {
//   unsigned long currentMillis = millis();
  
//   // Update LEDs every 10ms
//   if (currentMillis - lastLedUpdate >= 10) {
//     lastLedUpdate = currentMillis;
    
//     for (int i = 0; i < 3; i++) {
//       ledBrightness[i] += ledFadeDirection[i] * 5;
      
//       // Reverse direction if we hit the end of the brightness range
//       if (ledBrightness[i] <= 0) {
//         ledBrightness[i] = 0;
//         ledFadeDirection[i] = 1;
//       } else if (ledBrightness[i] >= 255) {
//         ledBrightness[i] = 255;
//         ledFadeDirection[i] = -1;
//       }
      
//       analogWrite(ledPins[i], ledBrightness[i]);
//     }
//   }
// }

/**
  Main program
*/
void loop() {
  readMidi();
  //updateLEDs();
}

//*******************************************************************************************************************
// Transmit MIDI Message
//*******************************************************************************************************************
void MIDI_TX(byte MESSAGE, byte PITCH, byte VELOCITY) {
  status1 = MESSAGE + midichannel;
  Serial.write(status1);
  Serial.write(PITCH);
  Serial.write(VELOCITY);
}
