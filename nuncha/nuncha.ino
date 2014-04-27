#include "Arduino.h"
#include "Nunchuck.h"
#include <Wire.h> 
#include <MozziGuts.h>

#define CONTROL_RATE 64 // powers of 2 please

#define LEFT 0
#define RIGHT 1
#define UP 2

int bumpy_input = 0;

int rollpitch = 0;

#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/triangle2048_int8.h> // triangle table for oscillator
#include <tables/saw2048_int8.h> // saw table for oscillator
#include <RollingAverage.h>

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);

// use: RollingAverage <number_type, how_many_to_average> myThing
// how_many_to_average has to be power of 2
RollingAverage <float, 8> rollLAverage;
RollingAverage <float, 8> rollRAverage;
RollingAverage <float, 8> pitchAverage;
RollingAverage <float, 4> xAverage;
RollingAverage <float, 4> yAverage;
int rollLeft;
int rollRight;
int pitch;
int xStick;
int yStick;

int play = 0;

void setup(){
  uint8_t error;
  
  error = Nunchuck.begin ();
  
  /*if ( error )
  {
    Serial.print   ( "can't initialize Nunchuck - error " );
    Serial.println ( error );
    while ( 1 );
  }*/
  
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  
  // set the frequency
  aSin.setFreq(440);
  aTri.setFreq(440);
  aSaw.setFreq(440);
}

void updateControl(){
   // read the input device

   Nunchuck.update(); 
   
   if ( !Nunchuck.getZButton() )
   { 
     bumpy_input = constrain(Nunchuck . getRoll(), -180, -1);
     bumpy_input = map(-bumpy_input, 0, 180, 0, 800);
     rollLeft = rollLAverage.next(bumpy_input);
     bumpy_input = constrain(Nunchuck . getRoll(), 0, 180);
     bumpy_input = map(bumpy_input, 0, 180, 0, 800);
     rollRight = rollRAverage.next(bumpy_input);
     bumpy_input = constrain(Nunchuck . getPitch(), 100, 174);
     bumpy_input = map(bumpy_input, 100, 174, 0, 800);
     pitch = pitchAverage.next(bumpy_input);

     aSin.setFreq(rollLeft);
     aTri.setFreq(rollRight);
     aSaw.setFreq(pitch);
     
     play = 1;
   }
   
   else{play = 0;}
}


int updateAudio(){
  if (play)
  {
    if ( pitch >= rollLeft)
    {
      if ( pitch >= rollRight)
      { return aSaw.next(); }
      else
      { return aTri.next(); }
    }
    else if ( pitch >= rollRight)
    {
      if ( pitch >= rollLeft)
      { return aSaw.next(); }
      else
      { return aSin.next(); }
    }
    else if ( rollRight >= rollLeft)
    {
      if ( pitch >= rollRight)
      { return aSaw.next(); }
      else
      { return aTri.next(); }
    }
    else if ( rollLeft >= rollRight)
    {
      if ( pitch >= rollLeft)
      { return aSaw.next(); }
      else
      { return aSin.next(); }
    }
  }
  
  return 0;
}

void loop(){
  audioHook();
}
