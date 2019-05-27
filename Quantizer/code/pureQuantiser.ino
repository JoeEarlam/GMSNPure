/*
  GMSN! Pure Quantiser v2.4
  08th May 2018
  cc-by 4.0
  Rob Spencer

  This version has:
  
    Solid lights, no flashing during playing.
    Remebers selection when power cycled.
    CV Thru, CV goes straight through when no LEDs selected, and LEDs display VU meter type behaviour, good for fault finding.
  
*/

#include "SPI.h"
#include <EEPROM.h>

//Setup LED Pin Varibles
const byte LR_0 = A3;
const byte LR_1 = A4;
const byte LR_2 = A5;
const byte LR_3 = 7;
const byte LC_0 = A0;
const byte LC_1 = A1;
const byte LC_2 = A2;

//Setup Button Pin Variable
const byte BC_0 = 2;
const byte BC_1 = 1;
const byte BC_2 = 0;
const byte BR_0 = 3;
const byte BR_1 = 4;
const byte BR_2 = 5;
const byte BR_3 = 6;

//Setup variable for the buttons, LEDs and temperament
int note;
int notes[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int equalTempered[13] = {0, 68, 136, 205, 273, 341, 409, 478, 546, 614, 682, 751, 819};


//Setup program control variables
int mode = 0;
int i = 1;

//Setup switch debounce variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;
int buttonState;
int lastButtonState[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//Setup SPI Bus Pin Variables
const byte csADC = 9;
const byte csDAC = 10;

//Setup CV and Trigger Pin Variables
int cvIn;
int cvOut;
int lastCvOut;
unsigned long lastCvChange;
int triggerOnTime = 10;
const byte trig = 8;

void setup() {

  //Setup LED Pins
  pinMode(LR_0, OUTPUT);
  pinMode(LR_1, OUTPUT);
  pinMode(LR_2, OUTPUT);
  pinMode(LR_3, OUTPUT);
  pinMode(LC_0, OUTPUT);
  pinMode(LC_1, OUTPUT);
  pinMode(LC_2, OUTPUT);

  digitalWrite(LR_0, HIGH);
  digitalWrite(LR_1, HIGH);
  digitalWrite(LR_2, HIGH);
  digitalWrite(LR_3, HIGH);
  digitalWrite(LC_0, LOW);
  digitalWrite(LC_1, LOW);
  digitalWrite(LC_2, LOW);

  //Setup Button Pins
  pinMode(BC_0, OUTPUT);
  pinMode(BC_1, OUTPUT);
  pinMode(BC_2, OUTPUT);
  pinMode(BR_0, INPUT);
  pinMode(BR_1, INPUT);
  pinMode(BR_2, INPUT);
  pinMode(BR_3, INPUT);

  digitalWrite(BC_0, LOW);
  digitalWrite(BC_1, LOW);
  digitalWrite(BC_2, LOW);

  //Setup SPI Chip Select Pins
  pinMode(csADC, OUTPUT);
  pinMode(csDAC, OUTPUT);

  digitalWrite(csADC, HIGH);
  digitalWrite(csDAC, HIGH);
  SPI.begin();

  //Setup Trigger Pin
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);

  //Read last saved notes from EEPROM
  for (int k = 0; k <= 12; k++) {
    if (EEPROM[k] == 255) {
      notes[k] = 0;
    } else {
      notes[k] = EEPROM[k];
    }
  }

}

void loop() {

  //The main loop() is made of two part.
  // 1) Button Management. Each loop through reads and lights one of the 12 buttons.
  // 2) CV Management. Each loop reads the CV In, then depending on the mode, will either Quantiser the CV and look after the Trigger, or will pass the CV straight through with no trigger.


  //1) Button Management
  
  //Read Buttons to see if they are being pressed in this cycle. As the program loops round it will check if a button us currently being pressed and turn on the relevant flag within the notes array.
  readNoteButton(i);

  //If the flag is set, light the relevant button.
  if (notes[i] == 1) {
    writeLED(i);
  } else {
    writeLED(0);
  }

  //Increment the counter so on the next loop we'll read and light the next LED.
  if (i == 12) {
    i = 0;
  } else {
    i++;
  }

  //This delay just helps with the brightness of the LEDs. If it wasn't there you wouldn't notice the accents.
  delay(1);


  //2) CV Management

  //Read CV In
  cvIn = adcRead(1);

  //Check all the note flags and if none are set, we're not quantising, mode=0  
  mode = 0;
  for (int j = 1; j <= 12; j++) {
    mode = mode + notes[j];
  }

  //If mode=0, just outout the input and flash the leds like a VU Meter. This is useful for fault finding. If no LEDs light, then issue is with the input ADC. If LEDs light, but no CV Out, issue is with output DAC.
  if (mode == 0) {
    cvOut = cvIn >> 1;
    dacWrite(cvOut);
    vuMeter();
  } else {

    //If not in mode=0, Quantise CV
    quantiseCV(cvIn);
    dacWrite(cvOut);

    //CV Out has changed, i.e. we've changed notes, then set the trigger out high.
    if (lastCvOut != cvOut) {
      digitalWrite(trig, HIGH);
      lastCvOut = cvOut;
      lastCvChange = millis();
    } else if ((millis() - lastCvChange) > triggerOnTime) {
      digitalWrite(trig, LOW);
    }
  }

  
}


void readNoteButton(byte button) {
  switch (button) {
    case 1:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_2, HIGH);
      note = digitalRead(BR_0);
      digitalWrite(BC_2, LOW);
      break;

    case 2:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_2, HIGH);
      note = digitalRead(BR_1);
      digitalWrite(BC_2, LOW);
      break;

    case 3:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_2, HIGH);
      note = digitalRead(BR_2);
      digitalWrite(BC_2, LOW);
      break;

    case 4:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_2, HIGH);
      note = digitalRead(BR_3);
      digitalWrite(BC_2, LOW);
      break;

    case 5:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_1, HIGH);
      note = digitalRead(BR_0);
      digitalWrite(BC_1, LOW);
      break;

    case 6:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_1, HIGH);
      note = digitalRead(BR_1);
      digitalWrite(BC_1, LOW);
      break;

    case 7:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_1, HIGH);
      note = digitalRead(BR_2);
      digitalWrite(BC_1, LOW);
      break;

    case 8:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_1, HIGH);
      note = digitalRead(BR_3);
      digitalWrite(BC_1, LOW);
      break;

    case 9:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_0, HIGH);
      note = digitalRead(BR_0);
      digitalWrite(BC_0, LOW);
      break;

    case 10:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_0, HIGH);
      note = digitalRead(BR_1);
      digitalWrite(BC_0, LOW);
      break;

    case 11:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_0, HIGH);
      note = digitalRead(BR_2);
      digitalWrite(BC_0, LOW);
      break;

    case 12:
      //Turn on BC_0 and read BR_0
      digitalWrite(BC_0, HIGH);
      note = digitalRead(BR_3);
      digitalWrite(BC_0, LOW);
      break;
  }

  //Debounce and toggle
  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (note != buttonState) {
      buttonState = note;
      if (buttonState == HIGH) {
        notes[button] = !notes[button];
        EEPROM[button] = notes[button];
        lastDebounceTime = millis();
      }
    }
  }
  lastButtonState[button] = notes[button];

}

void writeLED(byte LED) {
  switch (LED) {
    case 0:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, LOW);
      break;

    case 1:
      digitalWrite(LR_0, LOW);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, HIGH);
      break;

    case 2:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, LOW);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, HIGH);
      break;

    case 3:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, LOW);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, HIGH);
      break;

    case 4:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, LOW);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, HIGH);
      break;

    case 5:
      digitalWrite(LR_0, LOW);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, HIGH);
      digitalWrite(LC_2, LOW);
      break;

    case 6:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, LOW);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, HIGH);
      digitalWrite(LC_2, LOW);
      break;

    case 7:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, LOW);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, HIGH);
      digitalWrite(LC_2, LOW);
      break;

    case 8:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, LOW);
      digitalWrite(LC_0, LOW);
      digitalWrite(LC_1, HIGH);
      digitalWrite(LC_2, LOW);
      break;

    case 9:
      digitalWrite(LR_0, LOW);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, HIGH);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, LOW);
      break;

    case 10:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, LOW);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, HIGH);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, LOW);
      break;

    case 11:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, LOW);
      digitalWrite(LR_3, HIGH);
      digitalWrite(LC_0, HIGH);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, LOW);
      break;

    case 12:
      digitalWrite(LR_0, HIGH);
      digitalWrite(LR_1, HIGH);
      digitalWrite(LR_2, HIGH);
      digitalWrite(LR_3, LOW);
      digitalWrite(LC_0, HIGH);
      digitalWrite(LC_1, LOW);
      digitalWrite(LC_2, LOW);
      break;
  }
}

void dacWrite(int value) {

  digitalWrite(csDAC, LOW);
  byte data = value >> 8;
  data = data & B00001111;
  data = data | B00110000;
  SPI.transfer(data);

  data = value;
  SPI.transfer(data);

  digitalWrite(csDAC, HIGH);
}

int adcRead(byte channel) {

  byte commandbits = B00001101;          //command bits - 0000, start, mode, chn, MSBF
  unsigned int b1 = 0;                   // get the return var's ready
  unsigned int b2 = 0;
  commandbits |= ((channel - 1) << 1);   // update the command bit to select either ch 1 or 2
  digitalWrite(csADC, LOW);
  SPI.transfer(commandbits);             // send out the command bits
  const int hi = SPI.transfer(b1);       // read back the result high byte
  const int lo = SPI.transfer(b2);       // then the low byte
  digitalWrite(csADC, HIGH);
  b1 = lo + (hi << 8);                   // assemble the two bytes into a word
  //return b1;
  return (b1 >> 2);                      // We have got a 12bit answer but strip LSB's if
  // required >>4 ==10 bit (0->1024), >>2 ==12bit (0->4096)
}

int quantiseCV(int cvIn) {

  int octave = cvIn / 819;
  int vOct = octave * 819;
  int vOffset = 0;

  for (int index = 1; index <= 12; index ++) {
    if (notes[index] == 1 && (cvIn - vOct) > index * 63) {
      vOffset = (index * 63) - 63;
      cvOut = vOct + vOffset;
      //writeLED(index);
      cvOut = cvOut >> 1;
    }
  }

}

void vuMeter() {
  for (int l = 1; l <= 12; l++) {
    if (cvIn > (300 * l)) {
      writeLED(l);
    }
  }
}

