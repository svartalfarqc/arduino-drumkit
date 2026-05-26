// ============================================================
// Arduino Drumkit - Phase 1: Piezos + MIDI only
// ============================================================
// Based on original work by anouskadg / Jenna deBoisblanc / Spiekenzie Labs
// https://www.instructables.com/MIDI-Arduino-Drums/
// ============================================================

// ============================================================
// PIN ASSIGNMENTS
// ============================================================

char pinAssignments[5] = {A0, A1, A2, A3, A4}; // 5 piezo sensors

// ============================================================
// MIDI / PAD CONFIGURATION
// ============================================================

// Ugritone Kult Drums II — standard mapping
byte PadNote[5] = {
  48, // Tom1
  43, // Tom4
  38, // Snare
  51, // Ride
  57  // Crash2
};

// Minimum analog value to trigger a hit
int PadCutOff[5]   = {200, 200, 200, 200, 200};

// Debounce: loop cycles before a 2nd hit is allowed
int MaxPlayTime[5] = {60, 60, 60, 60, 60};

// true = velocity-sensitive, false = fixed 127
bool VelocityFlag = true;

// MIDI channel (0–15, i.e. ch.1–16 in DAW)
int midichannel = 0;

// ============================================================
// INTERNAL STATE
// ============================================================

bool activePad[5]  = {};
int PinPlayTime[5] = {};
byte status1;
int  hitavg;

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
}

// ============================================================
// MIDI TRANSMIT
// ============================================================

void MIDI_TX(byte MESSAGE, byte PITCH, byte VELOCITY) {
  status1 = MESSAGE + midichannel;
  Serial.write(status1);
  Serial.write(PITCH);
  Serial.write(VELOCITY);
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  for (int pin = 0; pin < 5; pin++) {
    hitavg = analogRead(pinAssignments[pin]);

    if (hitavg > PadCutOff[pin]) {
      if (!activePad[pin]) {
        // New hit
        int velocity;
        if (VelocityFlag) {
          velocity = constrain((hitavg / 8) - 1, 1, 127);
        } else {
          velocity = 127;
        }

        MIDI_TX(144, PadNote[pin], velocity); // Note ON
        PinPlayTime[pin] = 0;
        activePad[pin]   = true;

      } else {
        PinPlayTime[pin]++;
      }

    } else if (activePad[pin]) {
      PinPlayTime[pin]++;
      if (PinPlayTime[pin] > MaxPlayTime[pin]) {
        activePad[pin] = false;
        MIDI_TX(144, PadNote[pin], 0);        // Note OFF
      }
    }
  }
}