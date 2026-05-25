// ============================================================
// Arduino Drumkit - Enhanced with LED Modes
// ============================================================
// Based on original work by anouskadg / Jenna deBoisblanc / Spiekenzie Labs
// https://www.instructables.com/MIDI-Arduino-Drums/
//
// Enhancements:
//   - LED mode 0 (HIT):   Pad LED flashes on each drum hit
//   - LED mode 1 (LEARN): A random pad lights up — hit it to score!
//   - Binary score/channel display on 4 indicator LEDs
//   - Cleaner LED logic (no more 16-branch if-else)
//   - Button 1 (pin 2): Cycle LED mode
//   - Button 2 (pin 4): Context action (MIDI channel / game reset)
// ============================================================


// ============================================================
// PIN ASSIGNMENTS
// ============================================================

// Piezo sensor inputs
char pinAssignments[6] = {A0, A1, A2, A3, A4, A5};

// Pad LEDs — one per drum pad (all PWM-capable for future dimming)
//   Pad:   0   1   2   3   4   5
//   Pin:   3   5   6   9  10  11
const int padLedPins[6] = {3, 5, 6, 9, 10, 11};

// 4 indicator LEDs — display channel or score in binary (LSB = ledPin4)
const int ledPin1 = 7;
const int ledPin2 = 8;
const int ledPin3 = 12;
const int ledPin4 = 13;

// Control buttons (connect each button between pin and 5V;
// add a 10kΩ pull-down resistor between pin and GND)
const int modeButtonPin = 2;   // Cycles LED mode
const int altButtonPin  = 4;   // Context: MIDI channel (HIT) / reset game (LEARN)


// ============================================================
// MIDI / PAD CONFIGURATION
// ============================================================

// Ugritone Kult Drums II — standard mapping
byte PadNote1[6] = {
  52, // China1
  48, // Tom1
  43, // Tom4
  38, // Snare
  51, // Ride
  57  // Crash2
};

// "Gougoune mode" — kick-heavy alternate mapping
byte PadNote2[6] = {
  36, // Kick
  36, // Kick
  43, // Tom4
  38, // Snare
  51, // Ride
  57  // Crash2
};

int PadCutOff[6]   = {200, 200, 200, 200, 200, 200};
int MaxPlayTime[6] = {60,  60,  60,  60,  60,  60};

bool VelocityFlag = true;


// ============================================================
// INTERNAL STATE
// ============================================================

bool activePad[6]  = {};
int PinPlayTime[6] = {};
byte status1;
int  hitavg;

int modeButtonState = 0, lastModeButtonState = 0;
int altButtonState  = 0, altLastButtonState  = 0;

int controlState    = 0; // MIDI channel 0–15
int altControlState = 0; // 0 = PadNote1, 1 = PadNote2 (gougoune)


// ============================================================
// LED MODES
// ============================================================

#define MODE_HIT   0
#define MODE_LEARN 1
#define NUM_MODES  2

int ledMode = MODE_HIT;

int padLedTimer[6]      = {};
const int LED_ON_CYCLES = 25;

// Learning mode
int  learnTarget  = -1;
bool learnActive  = false;
unsigned long learnStartMs = 0;
const unsigned long LEARN_TIMEOUT_MS = 5000;
int  learnScore   = 0;

bool learnBlinking    = false;
int  learnBlinkTimer  = 0;
bool learnBlinkState  = true;
const int BLINK_TOTAL  = 50;
const int BLINK_PERIOD = 5;


// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);

  for (int i = 0; i < 6; i++) {
    pinMode(padLedPins[i], OUTPUT);
    digitalWrite(padLedPins[i], LOW);
  }

  pinMode(modeButtonPin, INPUT);
  pinMode(altButtonPin,  INPUT);

  randomSeed(analogRead(A3) ^ (analogRead(A4) << 8));

  updateIndicatorLEDs();

  // Startup flash — verify all pad LEDs are wired
  for (int i = 0; i < 6; i++) digitalWrite(padLedPins[i], HIGH);
  delay(300);
  for (int i = 0; i < 6; i++) digitalWrite(padLedPins[i], LOW);
}


// ============================================================
// INDICATOR LED HELPER
// ============================================================

void updateIndicatorLEDs() {
  int val = (ledMode == MODE_LEARN) ? learnScore : controlState;
  val &= 0x0F;
  digitalWrite(ledPin1, (val >> 3) & 1);
  digitalWrite(ledPin2, (val >> 2) & 1);
  digitalWrite(ledPin3, (val >> 1) & 1);
  digitalWrite(ledPin4, (val >> 0) & 1);
}


// ============================================================
// LEARNING MODE HELPERS
// ============================================================

void learnPickTarget() {
  if (learnTarget >= 0) digitalWrite(padLedPins[learnTarget], LOW);
  learnTarget     = random(0, 6);
  learnActive     = true;
  learnStartMs    = millis();
  learnBlinking   = false;
  learnBlinkTimer = 0;
  digitalWrite(padLedPins[learnTarget], HIGH);
}

void learnReset() {
  if (learnTarget >= 0) digitalWrite(padLedPins[learnTarget], LOW);
  learnTarget   = -1;
  learnActive   = false;
  learnScore    = 0;
  learnBlinking = false;
  updateIndicatorLEDs();

  for (int b = 0; b < 3; b++) {
    for (int i = 0; i < 6; i++) digitalWrite(padLedPins[i], HIGH);
    delay(100);
    for (int i = 0; i < 6; i++) digitalWrite(padLedPins[i], LOW);
    delay(100);
  }

  learnPickTarget();
}

void updateLearnMode() {
  if (!learnActive) return;

  if (learnBlinking) {
    learnBlinkTimer++;
    if (learnBlinkTimer % BLINK_PERIOD == 0) {
      learnBlinkState = !learnBlinkState;
      digitalWrite(padLedPins[learnTarget], learnBlinkState ? HIGH : LOW);
    }
    if (learnBlinkTimer >= BLINK_TOTAL) {
      learnBlinking   = false;
      learnBlinkTimer = 0;
      learnBlinkState = true;
      learnStartMs    = millis();
      digitalWrite(padLedPins[learnTarget], HIGH);
    }
    return;
  }

  if (millis() - learnStartMs > LEARN_TIMEOUT_MS) {
    learnBlinking   = true;
    learnBlinkTimer = 0;
      learnBlinkState = true;
  }
}


// ============================================================
// PUSH BUTTONS
// ============================================================

void readPushButtons() {
  modeButtonState = digitalRead(modeButtonPin);
  altButtonState  = digitalRead(altButtonPin);

  // Mode button: cycle LED mode
  if (lastModeButtonState == 0 && modeButtonState == 1) {
    for (int i = 0; i < 6; i++) {
      digitalWrite(padLedPins[i], LOW);
      padLedTimer[i] = 0;
    }
    learnBlinking = false;
    ledMode = (ledMode + 1) % NUM_MODES;

    if (ledMode == MODE_LEARN) {
      learnScore  = 0;
      learnTarget = -1;
      learnPickTarget();
    }
    updateIndicatorLEDs();
  }
  lastModeButtonState = modeButtonState;

  // Alt button: context action
  if (altLastButtonState == 0 && altButtonState == 1) {
    if (ledMode == MODE_HIT) {
      controlState = (controlState + 1) % 16;
      updateIndicatorLEDs();
    } else if (ledMode == MODE_LEARN) {
      learnReset();
    }
  }
  altLastButtonState = altButtonState;
}


// ============================================================
// PAD LED DECAY (HIT mode)
// ============================================================

void updatePadLeds() {
  for (int i = 0; i < 6; i++) {
    if (padLedTimer[i] > 0) {
      padLedTimer[i]--;
      if (padLedTimer[i] == 0) {
        digitalWrite(padLedPins[i], LOW);
      }
    }
  }
}


// ============================================================
// MIDI READ + LED RESPONSE
// ============================================================

void readMidi() {
  for (int pin = 0; pin < 6; pin++) {
    hitavg = analogRead(pinAssignments[pin]);

    if (hitavg > PadCutOff[pin]) {
      if (!activePad[pin]) {
        int velocity;
        if (VelocityFlag) {
          velocity = (hitavg / 8) - 1;
        } else {
          velocity = 127;
        }
        velocity = constrain(velocity, 1, 127);

        byte note = (altControlState == 0) ? PadNote1[pin] : PadNote2[pin];
        MIDI_TX(144, note, velocity);

        PinPlayTime[pin] = 0;
        activePad[pin]   = true;

        if (ledMode == MODE_HIT) {
          padLedTimer[pin] = LED_ON_CYCLES;
          digitalWrite(padLedPins[pin], HIGH);

        } else if (ledMode == MODE_LEARN) {
          if (learnActive && !learnBlinking) {
            if (pin == learnTarget) {
              // Correct hit!
              learnScore = min(learnScore + 1, 15);
              updateIndicatorLEDs();
              digitalWrite(padLedPins[learnTarget], LOW);
              learnActive = false;
              delay(200);
              learnPickTarget();
            } else {
              // Wrong pad — blink the correct one as a hint
              digitalWrite(padLedPins[learnTarget], LOW);
              delay(80);
              digitalWrite(padLedPins[learnTarget], HIGH);
              learnStartMs = millis(); // reset timeout
            }
          }
        }

      } else {
        PinPlayTime[pin]++;
      }

    } else if (activePad[pin]) {
      PinPlayTime[pin]++;
      if (PinPlayTime[pin] > MaxPlayTime[pin]) {
        activePad[pin] = false;
        byte note = (altControlState == 0) ? PadNote1[pin] : PadNote2[pin];
        MIDI_TX(144, note, 0);
      }
    }
  }
}


// ============================================================
// MIDI TRANSMIT
// ============================================================

void MIDI_TX(byte MESSAGE, byte PITCH, byte VELOCITY) {
  status1 = MESSAGE + controlState;
  Serial.write(status1);
  Serial.write(PITCH);
  Serial.write(VELOCITY);
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  readPushButtons();
  readMidi();

  if (ledMode == MODE_HIT) {
    updatePadLeds();
  } else if (ledMode == MODE_LEARN) {
    updateLearnMode();
  }
}