//Inspired by this instructable: https://www.instructables.com/MIDI-Arduino-Drums/
//Originally adapted from Jenna deBoisblanc and Spiekenzie Labs initial code
//https://www.instructables.com/Arduino-Xylophone/

// Adjustments made to original code:
// - Mapping to Ugritone Kult Drums II notes
// - Added a pushbutton to change pin to note mapping

//*******************************************************************************************************************
// User settable variables
//*******************************************************************************************************************
int pinRead;

char pinAssignments[6] ={
  A0,A1,A2,A3,A4,A5
};

// Ugritone Kult Drums II mapping

// Mapping with HiHat and Kick drum
byte PadNote1[6] = {
  44, // HiHat closed
  36, // Kick
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

// Mapping without HiHat and Kick drum
byte PadNote2[6] = {
  52, // China1
  48, // Tom1
  43, // Tom4
  38, // Snare
  51, // Ride
  57 // Crash2
  };
  

int PadCutOff[6] = 
{
//  400,400,400,400,400,400};  // Original code
  200,200,200,200,200,200};  // Minimum Analog value to cause a drum hit

int MaxPlayTime[6] = {
//  90,90,90,90,90,90};        // Original code
  60,60,60,60,60,60};        // Cycles before a 2nd hit is allowed
  
int midichannel = 0;      // MIDI channel from 0 to 15 (+1 in "real world")
boolean VelocityFlag  = true;   // Velocity ON (true) or OFF (false)

//*******************************************************************************************************************
// Internal Use Variables
//*******************************************************************************************************************
boolean activePad[6] = {
  0,0,0,0,0,0};     // Array of flags of pad currently playing
int PinPlayTime[6] = {
  0,0,0,0,0,0};     // Counter since pad started to play

byte status1;

int pin = 0;     
int hitavg = 0;

// pushbutton variables and constants
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
int buttonState = 0;         // variable for reading the pushbutton status
int lastButtonState = 0;
int controlState = 0;

//*******************************************************************************************************************
// Setup
//*******************************************************************************************************************
void setup() 
{
  Serial.begin(115200);   // connect to the serial port 115200
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
}
//*******************************************************************************************************************
// Main Program
//*******************************************************************************************************************
void loop() 
{
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  
  // pushbutton state change
  if (lastButtonState == 0 && buttonState == 1)
  {
    if (controlState == 15)
    {
      //digitalWrite(ledPin, HIGH);
      controlState = 0;
    }
    else
    {
      //digitalWrite(ledPin, LOW);
      controlState += 1;
    };
  }
  lastButtonState = buttonState;
  //delay(100);

  if (controlState == 0) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }
  
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

        // Set note depending on pushbutton state
        if (controlState == 0)
        {
          MIDI_TX(144,PadNote1[pin],hitavg); //note on
        }
        else
        {
          MIDI_TX(144,PadNote2[pin],hitavg); //note on
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

        // Set note depending on pushbutton state
        if (controlState == 0)
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
