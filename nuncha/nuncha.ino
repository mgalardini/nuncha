#include "Arduino.h"
#include "Nunchuck.h"
#include <Wire.h> 
#include <MozziGuts.h>

#define CONTROL_RATE 64 // powers of 2 please

#define LEFT 0
#define RIGHT 1
#define UP 2

int bumpy_input = 0;
int constrain_input = 0;

int rollpitch = 0;

#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/triangle2048_int8.h> // triangle table for oscillator
#include <tables/triangle_analogue512_int8.h> // saw table for oscillator

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE2048_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aSaw(TRIANGLE_ANALOGUE512_DATA);

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

unsigned char volumeLeft;
unsigned char volumeRight;
unsigned char volumePitch;

int play = 0;

void setup(){
  uint8_t error;
  
  error = Nunchuck.begin ();
  
  //Serial.begin(19200);
  
  /*if ( error )
  {
    Serial.print   ( "can't initialize Nunchuck - error " );
    Serial.println ( error );
    while ( 1 );
  }*/
  
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  
  // set the frequency
  aSin.setFreq(0);
  aTri.setFreq(0);
  aSaw.setFreq(0);
}

void updateControl(){
   // read the input device

   Nunchuck.update(); 
   
   //Serial.print(Nunchuck . getRaw()->yStick);
   //Serial.print(' ');
   //Serial.println((int)volume);
   
   if ( Nunchuck.getZButton() )
   { 
     constrain_input = constrain(Nunchuck . getRoll(), -180, -1);
     bumpy_input = map(-constrain_input, 0, 180, 0, 800);
     constrain_input = constrain(-constrain_input, 0, 100);
     volumeLeft = map(constrain_input, 0, 100, 0, 255);
     rollLeft = rollLAverage.next(bumpy_input);
     constrain_input = constrain(Nunchuck . getRoll(), 0, 180);
     bumpy_input = map(constrain_input, 0, 180, 0, 800);
     constrain_input = constrain(constrain_input, 0, 100);
     volumeRight = map(constrain_input, 0, 100, 0, 255);
     rollRight = rollRAverage.next(bumpy_input);
     constrain_input = constrain(Nunchuck . getPitch(), 100, 130);
     bumpy_input = map(constrain_input, 100, 174, 0, 800);
     constrain_input = constrain(constrain_input, 100, 144);
     volumePitch = map(constrain_input, 100, 144, 0, 150);
     pitch = pitchAverage.next(bumpy_input);
     
     constrain_input = constrain(Nunchuck . getRaw()->yStick, 31, 129);
     bumpy_input = map(-constrain_input, -129, -31, 200, 1000);
     yStick = yAverage.next(bumpy_input);     

     aSin.setFreq(pitch);
     aTri.setFreq(rollRight);
     aSaw.setFreq(rollLeft);
     
     play = 1;
   }
   
   else{play = 0;}
}


int updateAudio(){
  if (play)
  {
    int synth = (((int)aSaw.next() * volumeLeft) + ((int)aTri.next() * volumeRight) + ((int)aSin.next() * volumePitch))>>8;
    return synth;
  }
  
  return 0;
}

void loop(){
  audioHook();
}
