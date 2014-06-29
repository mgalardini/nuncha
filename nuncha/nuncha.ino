#include "Arduino.h"
#include "Nunchuck.h"
#include <Wire.h> 
#include <MozziGuts.h>
#include <ADSR.h>

#define CONTROL_RATE 128 // powers of 2 please
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/triangle2048_int8.h> // saw table for oscillator

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aRight(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aLeft(SIN2048_DATA);

Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aSin1(TRIANGLE2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aRight1(TRIANGLE2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aLeft1(TRIANGLE2048_DATA);

// Average the input signals
#include <RollingAverage.h>
// use: RollingAverage <number_type, how_many_to_average> myThing
// how_many_to_average has to be power of 2
RollingAverage <float, 4> rollLAverage;
RollingAverage <float, 4> rollRAverage;
RollingAverage <float, 16> pitchAverage;
#define  StickThresh  0.5f
int rollLeft;
int rollRight;
int pitch;

ADSR <CONTROL_RATE, CONTROL_RATE> envelope;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope2;

int zpressed = 0;
int cpressed = 0;
int znote = 0;
int cnote = 0;

unsigned int duration, attack, decay, sustain, release_ms;

byte gain = 0;
byte gain2 = 0;

void setup(){
  uint8_t error;
  
  error = Nunchuck.begin ();
  
  //Serial.begin(9600);
  
  /*if ( error )
  {
    Serial.print   ( "can't initialize Nunchuck - error " );
    Serial.println ( error );
    while ( 1 );
  }*/
  
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  
  // set the frequency
  aSin.setFreq(0);
  aRight.setFreq(0);
  aLeft.setFreq(0);
  aSin1.setFreq(0);
  aRight1.setFreq(0);
  aLeft1.setFreq(0);
  
  attack = 50;
  decay = 50;
  sustain = 200;
  release_ms = 400;
  envelope.setADLevels(255, 100);
  envelope.setTimes(attack,decay,sustain,release_ms);

  envelope2.setADLevels(255, 100);
  envelope2.setTimes(attack,decay,sustain,release_ms);
}

void updateControl(){

   Nunchuck.update(); 

   if( Nunchuck.getZButton() )
   {
     zpressed++;
     if( zpressed >= 1 )
     {
       envelope.noteOn();
       envelope2.noteOn();
       zpressed = 0;
       cnote = 0;
       znote = 1;
     }
   }
   else if( Nunchuck.getCButton() )
   {
     cpressed++;
     if( cpressed >= 1 )
     {
       envelope.noteOn();
       envelope2.noteOn();
       cpressed = 0;
       cnote = 1;
       znote = 0;
     }
       
   }
   float yStick = Nunchuck.getYStick();
   float xStick = Nunchuck.getXStick();  
 
   bool stickDn = ( yStick < - StickThresh ? true : false );
   bool stickUp = ( yStick >   StickThresh ? true : false );

   bool stickLf = ( xStick < - StickThresh ? true : false );
   bool stickRt = ( xStick >   StickThresh ? true : false );
   
     int Linput = constrain(Nunchuck . getRoll(), -180, -1);
     int BLinput = map(-Linput, 1, 180, 0, 400);
     rollLeft = rollLAverage.next(BLinput);

     int Rinput = constrain(Nunchuck . getRoll(), 0, 180);
     int BRinput = map(Rinput, 0, 180, 0, 400);
     rollRight = rollRAverage.next(BRinput);

     int constrain_input = constrain(Nunchuck . getPitch(), 18, 174);
     int bumpy_input = map(constrain_input, 18, 174, 440, 880);
     pitch = pitchAverage.next(bumpy_input);
   
   if ( stickUp )
   {
      pitch = pitch *2;
   }
   if ( stickDn )
   {
      pitch = pitch/4;
   }
   
   if ( stickLf )
   {
      pitch = pitch/2;
   }
   if ( stickRt )
   {
      pitch = pitch*2;
   }

     aSin.setFreq((int)pitch);   
     if (rollLeft > rollRight )
     {
       aRight.setFreq(0);
       aLeft.setFreq( rollLeft );
     }
     else
     {
       aLeft.setFreq(0);
       aRight.setFreq( rollRight );
     }
     
     aSin1.setFreq((int)pitch);   
     if (rollLeft > rollRight )
     {
       aRight1.setFreq(0);
       aLeft1.setFreq( rollLeft );
     }
     else
     {
       aLeft1.setFreq(0);
       aRight1.setFreq( rollRight );
     }

   envelope.update();
   envelope2.update();  
   
   gain = envelope.next();
   gain2 = envelope2.next();
}


int updateAudio(){
    int synth;

    if( rollLeft > rollRight )
    {
      if( cnote ){
        Q15n16 vibrato = (Q15n16) 200 * aLeft.next();
        synth = (long)gain * aSin.phMod(vibrato) >> 8;
      }
      else{
        Q15n16 vibrato = (Q15n16) 200 * aLeft1.next();
        synth = (long)gain * aSin1.phMod(vibrato) >> 8;
      }
    }
    else
    {
      if( cnote ){
        Q15n16 vibrato = (Q15n16) 200 * aRight.next();
        synth = (long)gain * aSin.phMod(vibrato) >> 8;
      }
      else{
        Q15n16 vibrato = (Q15n16) 200 * aRight1.next();
        synth = (long)gain * aSin1.phMod(vibrato) >> 8;
      }
    }
    return synth;
}

void loop(){
  audioHook();
}
