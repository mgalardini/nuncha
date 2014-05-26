#include "Arduino.h"
#include "Nunchuck.h"
#include <Wire.h> 
#include <MozziGuts.h>
#include <LowPassFilter.h>
#include <ADSR.h>

#define CONTROL_RATE 128 // powers of 2 please

int bumpy_input = 0;
int constrain_input = 0;

int rollpitch = 0;

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
RollingAverage <float, 16> rollLAverage;
RollingAverage <float, 16> rollRAverage;
RollingAverage <float, 16> pitchAverage;
RollingAverage <float, 4> xAverage;
RollingAverage <float, 4> yAverage;
int rollLeft;
int rollRight;
int pitch;
int xStick;
int yStick;

LowPassFilter lpf;

ADSR <CONTROL_RATE, CONTROL_RATE> envelope;
ADSR <CONTROL_RATE, CONTROL_RATE> envelope2;

unsigned char volumeLeft;
unsigned char volumeRight;
unsigned char volumePitch;

int pressed = 0;
int unpressed = 0;
int play = 0;

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

  lpf.setResonance(1);
  
  lpf.setCutoffFreq(128);
}

void updateControl(){
   Nunchuck.update(); 

   if( Nunchuck.getZButton() )
   {
     pressed++;
     if( pressed >= 1 )
     {
       envelope.noteOn();
       envelope2.noteOn();
       pressed = 0;
     }
   }

     constrain_input = constrain(Nunchuck . getRoll(), -180, -1);
     bumpy_input = map(-constrain_input, 0, 180, 11, 0);
     rollLeft = rollLAverage.next(bumpy_input);

     constrain_input = constrain(Nunchuck . getRoll(), 0, 180);
     bumpy_input = map(constrain_input, 0, 180, 11, 0);
     rollRight = rollRAverage.next(bumpy_input);

     constrain_input = constrain(Nunchuck . getPitch(), 18, 174);
     bumpy_input = map(constrain_input, 100, 150, 440, 880);
     pitch = pitchAverage.next(bumpy_input);
     
     constrain_input = constrain(Nunchuck . getRaw()->yStick, 31, 129);
     bumpy_input = map(-constrain_input, -129, -31, 200, 1000);
     yStick = yAverage.next(bumpy_input);     

     aSin.setFreq((int)pitch);
     if (rollRight == 0 )
     {
       aRight.setFreq(0);
     }
     else
     {
       aRight.setFreq((int)((2*pitch) + rollRight*73));
       //aRight.setFreq((int)((2*pitch) - rollRight*73));
     }
     if (rollLeft == 0 )
     {
       aLeft.setFreq(0);
     }
     else{
       aLeft.setFreq( (int)((pitch/2)+ 18*rollLeft));
       //aLeft.setFreq( (int)((pitch/2)- 18*rollLeft));
     }
   envelope.update();
   envelope2.update();  
 
   gain = envelope.next();
   gain2 = envelope2.next();
}


int updateAudio(){
    int synth;
    
    if( rollRight == 0 )
    {
      synth = lpf.next(((long)gain * aSin.next() + (int)gain2 * aLeft.next()) >> 9 );
    }
    else
    {
      synth = lpf.next(((long)gain * aSin.next() + (int)gain2 * aRight.next()) >> 9);
    }
    return synth;
}

void loop(){
  audioHook();
}
