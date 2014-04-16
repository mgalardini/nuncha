#include "Arduino.h"                                 // common Arduion constants and definitions
#include "Nunchuck.h"                                // Wii nunchuck library
#include <Wire.h> 
#include <MozziGuts.h>

#define CONTROL_RATE 64 // powers of 2 please

#define  StickThresh  0.2f                           // joystick threshold for change (0-1)

#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <RollingAverage.h>

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin0(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);

// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <float, 32> kAverage; // how_many_to_average has to be power of 2
int averaged;

unsigned char gain = 255;
float val = 0;

void setup(){
  uint8_t error;
  
  // initialize the Nunchuck
  
  Serial.begin ( 19200 );
  
  error = Nunchuck.begin ();
  
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  
  /*if ( error )
  {
    Serial.print   ( "can't initialize Nunchuck - error " );
    Serial.println ( error );
    while ( 1 );
  }*/
  
  aSin0.setFreq(440); // set the frequency
  aSin1.setFreq(440); // set the frequency  
}

void updateControl(){
  uint8_t error;

  static bool  lastStickUp = false;
  static bool  lastStickDn = false;
  static bool  lastStickLf = false;
  static bool  lastStickRt = false;
  
  
  // read the input device

  Nunchuck.update(); 
    
  // compute stick postion as a 4 way switch
    
   float xStick = Nunchuck.getXStick();
   float yStick = Nunchuck.getYStick();
   
   bool stickDn = ( yStick < - StickThresh ? true : false );
   bool stickUp = ( yStick >   StickThresh ? true : false );

   bool stickLf = ( xStick < - StickThresh ? true : false );
   bool stickRt = ( xStick >   StickThresh ? true : false );
   
   // in harmonizer mode ( no CButton ) use roll for note and use stick to control key changes
   
   if ( Nunchuck.getCButton() )
   { 
     //float bumpy_input = map(Nunchuck . getRoll(), -180, 180, 0, 800);
     float bumpy_input = map(Nunchuck . getPitch(), 0, 180, 0, 800);
     averaged = kAverage.next(bumpy_input);

     aSin0.setFreq(bumpy_input);
     aSin1.setFreq(averaged);
     //val = map(Nunchuck . getRoll(), -180, 180, -3, +3);
     //gain = gain + val;
     // use the 4 way switch to change key ( L/R = major / minor , U/D = major 3rd )
     //if ( stickUp && lastStickUp ) { gain = gain +3; }
     //if ( stickDn && lastStickDn ) { gain = gain -3; }
     //if ( stickUp && !lastStickUp ) { Synth.setKey ( Synth.getPendingKey()  + 5 );  Synth.setKeyMode ( 0 ); }
     //if ( stickDn && !lastStickDn ) { Synth.setKey ( Synth.getPendingKey()  - 5 );  Synth.setKeyMode ( 1 ); }
     //if ( stickLf && !lastStickLf ) { Synth.setKeyMode ( 1 ); }
     //if ( stickRt && !lastStickRt ) { Synth.setKeyMode ( 0 ); }
     
   }
   
   
   // in arpeggiator mode use roll for the stick to control the arpeggiator
      
   else
   {
     gain = gain - 3;
     // set the arpeggiator input with roll axis
 
     //Synth.setArpOctave ( -1.0f + 4.0f * Nunchuck . getRoll()  / 90.0f   );
  
  
      // L/R changes tempo,  U/D changes legato

     //if ( stickUp && !lastStickUp ) Synth.setArpLegato ( Synth.getArpLegato() + 0.1f );
     //if ( stickDn && !lastStickDn ) Synth.setArpLegato ( Synth.getArpLegato() - 0.1f );
     //if ( stickLf && !lastStickLf ) Synth.setArpTempo  ( Synth.getArpTempo()   - 10 );
     //if ( stickRt && !lastStickRt ) Synth.setArpTempo  ( Synth.getArpTempo()   + 10 );
     
   }

  
  // update shadow state
  
  lastStickUp = stickUp;
  lastStickDn = stickDn;
  lastStickLf = stickLf;
  lastStickRt = stickRt;

  //Nunchuck.printReal();
  //Serial.println();
}

/*
int updateAudio(){
  return (aSin.next()* gain)>>8; // return an int signal centred around 0
}
*/
int updateAudio(){
  //return 3*(aSin0.next()+aSin1.next())>>2;
  return (aSin1.next());
}

void loop(){
  audioHook();
}
