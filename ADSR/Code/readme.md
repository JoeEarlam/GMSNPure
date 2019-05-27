GMSN Pure ADSR v20170915
Rob Spencer 2017
cc-by 4.0

Full Open Source Documentation at https://gmsn.co.uk/pure-adsr including hardware files, Bill of Materials, Mouser Cart and Help Videos

For more indepth build and general support chat, join us at the GMSN! Forum: https://forum.gmsn.co.uk

The main loop() loops round continuously, everytime it goes round it calculates the current envelope value and writes it to the DAC.
There's some logic to figure out what Mode it's in and what phase it's in, which tell's it whether it should be calculating an attack value, etc.
Modes dependant on the Mode Switch. 1 = ADSR, 2 = AR, 3 = Looping Trapizoidal.
  
The flow through the phases are dependent on what mode the module is in.
  
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
