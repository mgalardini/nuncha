#include "Arduino.h"                                 // common Arduion constants and definitions
#include "Nunchuck.h"                                // Wii nunchuck library
#include <twi_nonblock.h> 
#include <MozziGuts.h>

#define CONTROL_RATE 64 // powers of 2 please

#define  StickThresh  0.5f                           // joystick threshold for change (0-1)


#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);


void setup(){
  uint8_t error;
  
  // initialize the Nunchuck
  
  error = Nunchuck.begin ();
  
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  
  /*if ( error )
  {
    Serial.print   ( "can't initialize Nunchuck - error " );
    Serial.println ( error );
    while ( 1 );
  }*/
  
  aSin.setFreq(440); // set the frequency
}

void updateControl(){
  /*uint8_t error;

  static bool  lastStickUp = false;
  static bool  lastStickDn = false;
  static bool  lastStickLf = false;
  static bool  lastStickRt = false;
  
  
  // read the input device

  //Nunchuck.update(); 
    
  // compute stick postion as a 4 way switch
    
   float xStick = Nunchuck.getXStick();
   float yStick = Nunchuck.getYStick();
   
   bool stickDn = ( yStick < - StickThresh ? true : false );
   bool stickUp = ( yStick >   StickThresh ? true : false );

   bool stickLf = ( xStick < - StickThresh ? true : false );
   bool stickRt = ( xStick >   StickThresh ? true : false );
   
   // in harmonizer mode ( no CButton ) use roll for note and use stick to control key changes
   
   if ( !Nunchuck.getCButton() )
   { 
         
     // use the 4 way switch to change key ( L/R = major / minor , U/D = major 3rd )

     if ( stickUp && !lastStickUp ) { Synth.setKey ( Synth.getPendingKey()  + 5 );  Synth.setKeyMode ( 0 ); }
     if ( stickDn && !lastStickDn ) { Synth.setKey ( Synth.getPendingKey()  - 5 );  Synth.setKeyMode ( 1 ); }
     if ( stickLf && !lastStickLf ) { Synth.setKeyMode ( 1 ); }
     if ( stickRt && !lastStickRt ) { Synth.setKeyMode ( 0 ); }
     
   }
   
   
   // in arpeggiator mode use roll for the stick to control the arpeggiator
      
   else
   {
     
     // set the arpeggiator input with roll axis
 
     Synth.setArpOctave ( -1.0f + 4.0f * Nunchuck . getRoll()  / 90.0f   );
  
  
      // L/R changes tempo,  U/D changes legato

     if ( stickUp && !lastStickUp ) Synth.setArpLegato ( Synth.getArpLegato() + 0.1f );
     if ( stickDn && !lastStickDn ) Synth.setArpLegato ( Synth.getArpLegato() - 0.1f );
     if ( stickLf && !lastStickLf ) Synth.setArpTempo  ( Synth.getArpTempo()   - 10 );
     if ( stickRt && !lastStickRt ) Synth.setArpTempo  ( Synth.getArpTempo()   + 10 );
     
   }

  
  // update shadow state
  
  lastStickUp = stickUp;
  lastStickDn = stickDn;
  lastStickLf = stickLf;
  lastStickRt = stickRt;
  */
}

int updateAudio(){
  return aSin.next(); // return an int signal centred around 0
}

void loop(){
  audioHook();
}
