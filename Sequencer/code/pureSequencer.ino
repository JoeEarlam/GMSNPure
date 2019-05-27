int internal_clock_rate_RAW;
int internal_clock_rate;
int trigLength = 50, trigLengthVar;
byte Direction = 0;
boolean forwardBackward = true;
boolean clockHigh = false;
boolean resetFlag = true, buttonFlag = true;
byte STEP = 0;

boolean PLAY = true;

boolean clkReadFlag = true;
int sweepCompare;

unsigned long cMillis, pMillis = 0, clkMillis = 0;

void setup(){
  //gates 0-5 outputs port c
  DDRC = B111111;
  DDRB = B111111;
  
  //Clock in
  pinMode(2, INPUT);
  pinMode(10, OUTPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(4, INPUT);
  pinMode(5, OUTPUT);
  
  /*
  DDRD = B11111110;
  pinMode(8, OUTPUT);
  //digitalWrite(8, HIGH); 
  pinMode(9, OUTPUT);
  //digitalWrite(9, HIGH); 
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH); 
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);
  */
  pMillis = millis();

  trigLengthVar = trigLength;
}


void loop(){
  directionToggle();
  if(digitalRead(4) == HIGH && buttonFlag){
    //PORTB |= (1 << 1);
    PORTD |= (1 << 5);
    PLAY = false;
    buttonFlag = false;
    sweepCompare = (1023 - analogRead(A7));
  }
  if(digitalRead(4) == LOW && !buttonFlag){
    //PORTB &= ~(1 << 1);
    PORTD &= ~(1 << 5);
    buttonFlag = true;
  }

  while(!PLAY){
    if(digitalRead(4) == LOW && !buttonFlag){
      //PORTB &= ~(1 << 1);
      buttonFlag = true;
      delay(20);
    }
    if(digitalRead(4) == HIGH && buttonFlag){
      buttonFlag = false;
      PLAY = true;
    }
    
    if(abs((1023 - analogRead(A7)) - sweepCompare) >= 20){
      sweepCompare = 1024;
    }
    if(sweepCompare == 1024){
      STEP = (1023 - analogRead(A7))/129;
      muxGate(STEP);
    muxPot(STEP);
    }
  }

  if(digitalRead(3) == LOW && resetFlag){
    if(Direction == 1){
      //PORTB &= ~1;
      PORTB &= ~(1 << 2);
      //PORTB |= (1 << 2);
      pMillis = cMillis - internal_clock_rate;
      STEP = 8;
    }
    else if(Direction == 0){
      //PORTB &= ~1;
      PORTB &= ~(1 << 2);
      //PORTB |= (1 << 2);
      pMillis = cMillis - internal_clock_rate;
      STEP = random(8);
    }
    else{
      pMillis = cMillis - internal_clock_rate;
      forwardBackward = !forwardBackward;
    }

    resetFlag = false;
  }
  if(digitalRead(3) == HIGH && !resetFlag){
    resetFlag = true;
  }

  cMillis = millis();

  if(clockHigh){
    if(cMillis - clkMillis >= trigLengthVar){
      clockHigh = false;
      //PORTB &= ~1;
      PORTB &= ~(1 << 2);
      //PORTB |= (1 << 2);
    }
  }

  internal_clock_rate_RAW = (1023 - analogRead(A7));
  internal_clock_rate = fscale( 0, 1023, 1500, 7, internal_clock_rate_RAW, 10);

  if(internal_clock_rate < (trigLength * 2)){
    trigLengthVar = internal_clock_rate/2;
  }
  else{
    trigLengthVar = trigLength;
  }


  if(cMillis - pMillis >= internal_clock_rate){
    pMillis = cMillis;

    clockHigh = true;
    clkMillis = cMillis;
    //PORTB |= 1;
    PORTB |= (1 << 2);
    //PORTB &= ~(1 << 2);

    muxGate(STEP);
    muxPot(STEP);
  }

  if(digitalRead(2) == LOW && clkReadFlag){

    if(Direction == 1){
      //forward
      STEP++;
      if(STEP > 7){
        STEP = 0;
      }
      muxGate(STEP);
      muxPot(STEP);
    }
    else if(Direction == 0){
      //Random
      STEP = random(8);
      muxGate(STEP);
      muxPot(STEP);
    }
    else if(Direction == 2){
      //Pen
      if(forwardBackward){
        STEP++;
        if(STEP > 7){
          forwardBackward = false;
          STEP--;
        }
      }
      else{
        STEP--;
        if(STEP > 8){
          forwardBackward = true;
          STEP++;
        }
      }
      muxGate(STEP);
      muxPot(STEP);


    }


    clkReadFlag = false;
  }
  if(digitalRead(2) == HIGH && !clkReadFlag){

    clkReadFlag = true;
  }

}


void directionToggle(){
  
  if(digitalRead(7) == HIGH){
    Direction = 1;
  }
  else if(digitalRead(6) == HIGH){
    Direction = 2;
  }
  else{
    Direction = 0;
  }

}




