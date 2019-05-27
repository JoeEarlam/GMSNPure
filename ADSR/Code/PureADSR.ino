/*

GMSN Pure ADSR v20170915
Rob Spencer 2017
cc-by 4.0

Full Open Source Documentation at https://gmsn.co.uk/pure-adsr including hardware files, Bill of Materials, Mouser Cart and Help Videos

For more indepth build and general support chat, join us at the GMSN! Synth Design Slack: https://join.slack.com/t/gmsnsynthdesign/shared_invite/MjE0NzM1ODc3NDkyLTE1MDA0NTI1MTItODQ3MDM4OTdlYw

*/

#include "SPI.h"

//Setup pin variables
const byte dacCS = 10;
const byte trigIn = 2;
const byte trigBut = 4;
const byte modeSw1 = 6;
const byte modeSw2 = 7;

const byte knob1 = A3;
const byte knob2 = A2;
const byte knob3 = A1;
const byte knob4 = A0;

//Setup envelope variable
float aPot, aCoeff, enVal = 0, dPot, dCoeff, sPot, sCoeff, sVal, rPot, rCoeff, onPot, offPot;

//Setup state variables
boolean gate = 0, rising = 0;
byte mode, phase = 0;
int trapOnCount = 0, trapOffCount = 0;

//Setup switch debounce variables
int buttonState, lastButtonState = HIGH;
unsigned long lastDebounceTime = 0, debounceDelay = 50;


void setup() {

  //Start DAC Comms
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);

  //Configure Pins
  pinMode(dacCS, OUTPUT); //DAC CS
  pinMode(trigIn, INPUT); //TRIGGER IN
  pinMode(trigBut, INPUT); //Button
  pinMode(modeSw1, INPUT); //MODE SW1
  pinMode(modeSw2, INPUT); //MODE SW2
  digitalWrite(dacCS, HIGH);

  /*Attach Interupt for Trigger / Gate in. The button also calls this routine but is polled in the main loop() rather than attached to an interrupt, 
  the hardware design does have it connected to an interrupt pin as well, but during an Interrupt call the millis() function is disabled, which interferes with the debouce routine,
  hence having the button polled*/
  attachInterrupt(digitalPinToInterrupt(trigIn), gateOn, FALLING);

}

void loop() {

  /*The main loop() llops round continuously, everytime it goes round it calculates the current envelope value and writes it to the DAC.
    There's some logic to figure out what Mode it's in and what phase it's in, which tell's it whether it should be calculating an attack value, etc.
    Modes dependant on the Mode Switch. 1 = ADSR, 2 = AR, 3 = Looping Trapizoidal.*/
  
  //Poll the Mode switch to set the mode
  if (digitalRead(modeSw2) == HIGH) {
    mode = 1;
  } else if (digitalRead(modeSw1) == LOW && digitalRead(modeSw2) == LOW) {
    mode = 2;
  } else {
    mode = 3;
  }

  /* The flow through the phases are dependent on what mode the module is in.
  ADSR:
    Phase 1 = Attack
    Phase 2 = Decay to sustain
    Phase 3 = Release
    
    In this mode, the lack of a Gate In or the Button not being down, moves the modules into the Release Phase.
    So in performance terms, a Gate On moves through Attack, Decay and holds at the Sustain Level.
    When the Gate is removed, say by taking a finger off the key on the keyboard, the envelope moves into Release.
    
    This is the only mode which use the Gate, the other two treat the leading edge of the Gate as a Trigger.
    Any new Gate or Trigger will restart the envelope from zero.
  
  AR:
    Phase 1 = Attack
    Phase 3 = Release
    
    The leading edge of a Gate or Trigger starts the Attack phase, once the envelope reaches the max value, it moves into the Release Phase.
    The Gate or Trigger has no other function, accept to start the envelope running. Any new Gate or Trigger will restart the envelope from zero.
    
  Trap:
    Phase 1 = Attack
    Phase 4 = On
    Phase 3 = Release
    Phase 5 = Off
    
    The phase numbers are intetionally out of sequence. Ideally the Trap phases would be 4, 5, 6 & 7, however the Attack and Release are re-used, so it made sense to do it this way.
    
    The Trap mode loops round, so it can be shaped as some sort of weird LFO, all phases can be taken down to ultashort and snappy or ultralong, over 8 minutes for each phase.
    With all controls set to minimum, the envelope will cycle so quickly it is in the audio range, so crazy waveforms can be created. Set all controls to near maximum and long plus 30 min 
    envelopes can be used for evolving patches. 
   
   Phase Control Logic
     The mechanics of each phase have been kept seperate from the control logic, with each phase having it's own function.
     The control logic is a simple switch() statement that reads which phase the envelope is in, enters that function, does the envelope value calculation and writes the value to the DAC.
     
    */
    
    
    //Control logic which calls the functions
  switch (phase) {
    
    //Phase 0 is idle, but if it's in the Trap mode it kicks it back into the Attack phase.
    case 0:
      if (mode == 3){
        phase = 1;
      }
      break;
      
    //Attack Phase with some logic to kick it into the correct next phase depending on the Mode.
    case 1:
      attack();

      if (enVal >= 4095) {
        switch (mode) {
          case 1:
            phase = 2;
            break;

          case 2:
            phase = 3;
            break;

          case 3:
            phase = 4;
            break;
        }
      }

      if (mode == 1 && digitalRead(trigBut) == HIGH && digitalRead(trigIn) == HIGH) {
        phase = 3;
      }
      break;

    //Decay to Sustain Phase.
    case 2:
      decaySustain();
      if (mode == 1 && digitalRead(trigBut) == HIGH && digitalRead(trigIn) == HIGH) {
        phase = 3;
      }
      break;

    //Release Phase. If it's in ADSR or AR mode, then knob 4 is the Release. If it's in Trap mode, then it's knob 3.
    case 3:
      if (mode == 3) {
        releasePhase(knob3);
      } else {
        releasePhase(knob4);
      }
      if (enVal < 100) {
        enVal = 0;
        if (mode == 3) {
          phase = 5;
        } else {
          phase = 0;
        }
      }
      break;

    //Phase 4, Trap On
    case 4:
      trapOn();
      break;

    //Phase 5, Trap Off
    case 5:
      trapOff();
      break;
  }


  //Poll Trigger Button, debounce, initialise Up phase
  trigButton();
}

void attack() {
  aPot = fscale(0, 1024, 3000, 0, analogRead(knob1), 10);
  //aPot = map(analogRead(knob1), 0, 1024, 4096, 0);
  if (aPot > 2999) {
    enVal = 4095;
  } else {
    float aCoeff = aPot / 16384;
    enVal += aCoeff * (4095 - enVal);
    if (enVal > 4090) {
      enVal = 4095;
    }
  }
  mcpWrite((int)enVal);
}

void decaySustain() {
  dPot = fscale(0, 1024, 1024, 0, analogRead(knob2), 10);
  float dCoeff = dPot / 16384;
  sPot = map(analogRead(knob3), 0, 1024, 0, 4096);
  enVal += dCoeff * (sPot - enVal);
  mcpWrite((int)enVal);
}

void releasePhase(byte knob) {
  rPot = fscale(0, 1024, 100, 0, analogRead(knob), 10);
  //rPot = map(analogRead(A0), 0, 1024, 80, -0);
  //float rCoeff = rPot / 16384;
  float rCoeff = rPot / 1024;
  enVal += rCoeff * (1 - enVal);
  mcpWrite((int)enVal);
}

void trapOn() {
  mcpWrite((int)enVal);
  onPot = fscale(0, 1024, 0, 30000, analogRead(knob2), -7);
  if (trapOnCount >= onPot) {
    trapOnCount = 0;
    phase = 3;
  } else {
    trapOnCount++;
  }
}

void trapOff() {
  mcpWrite((int)enVal);
  offPot = fscale(0, 1024, 0, 30000, analogRead(knob4), -7);
  if (trapOffCount >= offPot) {
    trapOffCount = 0;
    phase = 1;
  } else {
    trapOffCount++;
  }
}


/*Debounce routine for the switch. Switches are very noises when they are switching.
When you press a switch, the mechanism can "bounce" on, off, on, off, on, etc, in the space of a few milliseconds.
This debounce function removes the noise.*/
void trigButton() {
  int reading = digitalRead(trigBut);

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        gateOn();
        lastDebounceTime = millis();
      }
    }
  }
  lastButtonState = reading;
}

//Interrupt routine for rising edge of Gate
void gateOn() {
  if (phase == 1){
    enVal = 1;
  } else {
    phase = 1;
  }
  trapOffCount = 0;
  trapOnCount = 0;
}

/*Writing to the DAC.
The whole purpose of the module is to output a voltage envelope. That voltage then controls another module, such as a VCA or a filter.
In order to get that voltage out, we need to convert the value of the "envValue" variable into a voltage, which is done by a Digital to Analogue Converter, or DAC for short.
The function below send envValue to the DAC for conversion to the analogue voltage.
0 = Off 4095 = Full on.*/
void mcpWrite(int value) {

  //CS
  digitalWrite(10, LOW);

  //DAC1 write

  //set top 4 bits of value integer to data variable
  byte data = value >> 8;
  data = data & B00001111;
  data = data | B00110000;
  SPI.transfer(data);

  data = value;
  SPI.transfer(data);

  // Set digital pin 10 HIGH
  digitalWrite(10, HIGH);
}
