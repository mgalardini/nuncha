#include "Arduino.h"
#include "Nunchuck.h"
#include <Wire.h> 
#include <MozziGuts.h>
#include <ADSR.h>

#define CONTROL_RATE 128 // powers of 2 please
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aRight(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aLeft(SIN2048_DATA);

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

ADSR <CONTROL_RATE, CONTROL_RATE> envelope3;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope4;

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
  
  attack = 50;
  decay = 50;
  sustain = 100;
  release_ms = 500;
  envelope.setADLevels(255, 100);
  envelope.setTimes(attack,decay,sustain,release_ms);

  envelope2.setADLevels(255, 100);
  envelope2.setTimes(attack,decay,sustain,release_ms);

  attack = 100;
  decay = 200;
  sustain = 500;
  release_ms = 1000;
  envelope3.setADLevels(255, 100);
  envelope3.setTimes(attack,decay,sustain,release_ms);

  envelope4.setADLevels(255, 100);
  envelope4.setTimes(attack,decay,sustain,release_ms);
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
       envelope3.noteOn();
       envelope4.noteOn();
       cpressed = 0;
       cnote = 1;
       znote = 0;
     }
       
   }
   float yStick = Nunchuck.getYStick();
   
   bool stickDn = ( yStick < - StickThresh ? true : false );
   bool stickUp = ( yStick >   StickThresh ? true : false );

     int Linput = constrain(Nunchuck . getRoll(), -180, -1);
     int BLinput = map(-Linput, 1, 180, 1, 11);
     rollLeft = rollLAverage.next(BLinput);

     int Rinput = constrain(Nunchuck . getRoll(), 0, 180);
     int BRinput = map(Rinput, 0, 180, 1, 11);
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
      pitch = pitch/2;
   }

     aSin.setFreq((int)pitch);   
     if (rollLeft > rollRight )
     {
       aRight.setFreq(0);
        aLeft.setFreq( (int)((pitch/2) + 18*rollLeft));
     }
     else
     {
       aLeft.setFreq(0);
       aRight.setFreq((int)((pitch/2) + 18*rollRight));
     }
   envelope.update();
   envelope2.update();  
   envelope3.update();
   envelope4.update();  
   
   if( znote )
   {
     gain = envelope.next();
     gain2 = envelope2.next();
   }
   else
   {
     gain = envelope3.next();
     gain2 = envelope4.next();
   }

}


int updateAudio(){
    int synth;
    
    if( rollLeft > rollRight )
    {
      synth = ((long)gain * aSin.next() + (int)gain2 * aLeft.next()) >> 9 ;
    }
    else
    {
      synth = ((long)gain * aSin.next() + (int)gain2 * aRight.next()) >> 9;
    }
    return synth;
}

void loop(){
  audioHook();
}
