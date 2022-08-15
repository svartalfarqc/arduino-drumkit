//Inspired by this instructable by user anouskadg: https://www.instructables.com/MIDI-Arduino-Drums/
//Originally adapted from Jenna deBoisblanc and Spiekenzie Labs initial code
//https://www.instructables.com/Arduino-Xylophone/

// Adjustments made to original code:
// - Mapping to Ugritone Kult Drums II notes (https://ugritone.com/products/kvlt-drums-ii)
// - Added a pushbutton to change pin to note mapping
// - Added a pushbutton to change output from midi channel 1 to 16
// - Added 4 LEDs to show currently selected midi channel in binary

//*******************************************************************************************************************
// User settable variables
//*******************************************************************************************************************
int pinRead;

char pinAssignments[6] ={A0,A1,A2,A3,A4,A5};

// Ugritone Kult Drums II mapping

// Mapping WITHOUT HiHat and Kick drum
byte PadNote1[6] = {
  52, // China1
  48, // Tom1
  43, // Tom4
  38, // Snare
  51, // Ride
  57 // Crash2
  
  // NOTE BANK
  //32, // HiHat
  //36, // Kick
  //41, // Tom5
  //42, // HiHat Medium
  //43, // Tom4
  //44, // HiHat closed
  //45, //Tom3
  //46, // HiHat open
  //47, // Tom2
  //48, // Tom1
  //49, // Crash1
  //50, // Crash1 quick
  //52, // China1
  //53, // Ride quick
  //54, // Ride crash
  //55, // Splash1
  //56, // FxCymbal1
  //58, // Crash2 quick
  //59, // FxCymbal2
  //60, // China2
  };         // MIDI notes from 0 to 127 (Mid C = 60)

// Mapping WITH Kick drum (gougoune mode)
// https://www.je-parle-quebecois.com/lexique/definition/gougoune.html
byte PadNote2[6] = {
  36, // Kick
  36, // Kick
  43, // Tom4
  38, // Snare
  51, // Ride
  57 // Crash2
  };
  
int PadCutOff[6] = {200,200,200,200,200,200};  // Minimum Analog value to cause a drum hit (original code was 400)
int MaxPlayTime[6] = {60,60,60,60,60,60};        // Cycles before a 2nd hit is allowed (original code says 90)
  
int midichannel = 0;      // MIDI channel from 0 to 15 (+1 in "real world")
boolean VelocityFlag  = true;   // Velocity ON (true) or OFF (false)

//*******************************************************************************************************************
// Internal Use Variables
//*******************************************************************************************************************
boolean activePad[6] = {0,0,0,0,0,0};     // Array of flags of pad currently playing
int PinPlayTime[6] = {0,0,0,0,0,0};     // Counter since pad started to play

byte status1;
int pin = 0;     
int hitavg = 0;

// pushbutton variables and constants
const int buttonPin = 2;     // the number of the pushbutton pin
const int altButtonPin = 4;     // the number of the pushbutton pin
const int ledPin1 =  7;      // the number of the LED pin
const int ledPin2 =  8;      // the number of the LED pin
const int ledPin3 =  12;      // the number of the LED pin
const int ledPin4 =  13;      // the number of the LED pin

// pushbutton to select midi channel
int buttonState = 0;
int lastButtonState = 0;
int controlState = 0;

// pushbutton to select "alt" mode (alternate pin mapping) a.k.a. gougoune mode
int altButtonState = 0;
int altLastButtonState = 0;
int altControlState = 0;

/**
  Setup
*/
void setup() 
{
  Serial.begin(115200);   // connect to the serial port 115200
  
  // initialize the LED pins as an output:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  // initialize the pushbutton pins as an input:
  pinMode(buttonPin, INPUT);
  pinMode(altButtonPin, INPUT);
}

/**
  Read pushbuttons and adjust LEDs as needed
*/
void readPushButtons(){
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  altButtonState = digitalRead(altButtonPin);
  
  // pushbutton state change
  if (lastButtonState == 0 && buttonState == 1)
  {
    if (controlState == 15)
    {
      controlState = 0;
    }
    else
    {
      controlState += 1;
    };
  }
  lastButtonState = buttonState;

  // alt pushbutton state change
  if (altLastButtonState == 0 && altButtonState == 1)
  {
    if (altControlState == 0)
    {
      altControlState = 1;
    }
    else
    {
      altControlState = 0;
    };
  }
  altLastButtonState = altButtonState;

  // adjust LEDs to show current pushbutton state
  if (controlState == 0) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 1){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 2){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 3){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 4){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 5){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 6){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 7){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 8){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 9){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 10){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 11){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 12){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 13){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, HIGH);
  }
  else if (controlState == 14){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, LOW);
  }
  else if (controlState == 15){
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin4, HIGH);
  }
}

/**
  Read input pads and send midi signals
*/
void readMidi(){
  for(int pin=0; pin < 6; pin++)                                             
  {
    hitavg = analogRead(pinAssignments[pin]);  

    // read the input pin
    if((hitavg > PadCutOff[pin]))
    {
      if((activePad[pin] == false))
      {
        if(VelocityFlag == true)
        {
          //hitavg = 127 / ((1023 - PadCutOff[pin]) / (hitavg - PadCutOff[pin]));    // With full range (Too sensitive ?)
          hitavg = (hitavg / 8) -1 ;                                                 // Upper range
        }
        else
        {
          hitavg = 127;
        }

        // Set note mapping depending on alt pushbutton state
        if (altControlState == 0)
        {
          MIDI_TX(144,PadNote1[pin],hitavg);
        }
        else
        {
          MIDI_TX(144,PadNote2[pin],hitavg);
        }
        
        PinPlayTime[pin] = 0;
        activePad[pin] = true;
      }
      else
      {
        PinPlayTime[pin] = PinPlayTime[pin] + 1;
      }
    }
    else if((activePad[pin] == true))
    {
      PinPlayTime[pin] = PinPlayTime[pin] + 1;
      if(PinPlayTime[pin] > MaxPlayTime[pin])
      {
        activePad[pin] = false;

        // Set note mapping depending on alt pushbutton state
        if (altControlState == 0)
        {
          MIDI_TX(144,PadNote1[pin],0); 
        }
        else
        {
          MIDI_TX(144,PadNote2[pin],0); 
        }
      }
    }
  } 
}

/**
  Main program
*/
void loop() 
{
  // read pushbuttons and adjust LEDs as needed
  readPushButtons();

  // read input pads and send midi signals
  readMidi();
}

//*******************************************************************************************************************
// Transmit MIDI Message
//*******************************************************************************************************************
void MIDI_TX(byte MESSAGE, byte PITCH, byte VELOCITY) 
{
  midichannel = controlState;
  status1 = MESSAGE + midichannel;
  Serial.write(status1);
  Serial.write(PITCH);
  Serial.write(VELOCITY);
}
