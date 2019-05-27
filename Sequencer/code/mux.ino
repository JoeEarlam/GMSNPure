void muxGate(byte channel){
  
  if(channel < 6){
    PORTC = (1 << channel);
    
    PORTB = PORTB & B111100;
    PORTB = PORTB | B000000;
  }else if(channel == 6){
    PORTC = 0;
    PORTB = PORTB & B111100;
    PORTB = PORTB | B000001;
  }else{
    PORTC = 0;
    PORTB = PORTB & B111100;
    PORTB = PORTB | B000010;
  }
  
  //PORTD = PORTD & B11100011;
  //PORTD = PORTD | (channel << 2);
}

void muxPot(byte channel){
  PORTB = PORTB & B000111;
  PORTB = PORTB | (channel << 3);
  
//  PORTD = PORTD & B00011111;
//  PORTD = PORTD | (channel << 5);
}
