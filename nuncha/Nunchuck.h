#ifndef _NUNCHUCK_H_
#define _NUNCHUCK_H_

// ------------------------------------------------------------------------------------------------------
// Nunchuck - a simple library to read a Wii compatible Nunchuck
// ------------------------------------------------------------------------------------------------------
//
// Please refer to the accompnying source file for details on how to connect this up, what its made of,
// what it does, and how you can use it. Finding out how to break it is left up to the reader.
//
// ------------------------------------------------------------------------------------------------------



// Nunchuck I2C data protocol

enum
{
   NCStickX  = 0 ,                          // joystick X axis position
   NCStickY  = 1 ,                          // joystick Y axis position
   NCAccelX  = 2 ,                          // acclerometer x axis high 8 bits
   NCAccelY  = 3 ,                          // acclerometer y axis high 8 bits
   NCAccelZ  = 4 ,                          // acclerometer z axis high 8 bits
   NCButtons = 5 ,                          // bit 0   = Z button
                                            // bit 1   = C button
                                            // bit 2|3 = low 2 bits of X acceleromater
                                            // bit 4|5 = low 2 bits of Y accelerometer
                                            // bit 6|7 = low 2 bits of Z acclerometer
   NCDataSize               
};



// Nunchuck raw data

typedef struct 
{
  
  // raw digital data
  
  uint8_t  xStick;                          // X axis joystick X value ( -127 to 127 )
  uint8_t  yStick;                          // Y axis joystick Y value ( -127 to 127 )
  
  bool     zButton;                         // Z button status ( true or false )
  bool     cButton;                         // C button status ( true of false ) 
  
  uint16_t xAccel;                          // X axis acceleration ( -1023 to 1023 )
  uint16_t yAccel;                          // Y axis acceleration ( -1023 to 1023 )
  uint16_t zAccel;                          // Z axis acceleration ( -1023 to 1023 )
  
  
  // converted floating point data
  
  float    xSReal;                          // X axis stick ( -1.0 left to 1.0 right )
  float    ySReal;                          // Y axis stick ( -1.0 left to 1.0 right )
  
  float    xAReal;                           // X axis acceleration in Gs
  float    yAReal;                           // Y axis acceleration in Gs
  float    zAReal;                           // Z axis acceleration in Gs
  float    rAReal;                           // acceleration magnitude in Gs

  float    rollReal;                         // roll rotation in degrees
  float    pitchReal;                        // pitch rotation in degrees
  
} NCData;


// Nunchuck interface class

class NunchuckInterface
{
  
  public:
  
   uint8_t     begin();                     // intialize connection with the nunchuck
   uint8_t     update();                    // update the status of the nunchuck
   void        printRaw();                  // print the raw sample data to the serial monitor
   void        printReal();                 // print the computed data in real units
   
   NCData *    getRaw();                    // get the raw sampled data from the last update

   float       getXStick();                 // get x stick position ( -1 = left , +1 = right )
   float       getYStick();                 // get y stick position ( -1 = lett , +1 = right )
   float       getXAccel();                 // get x acceleration ( in Gs , positive body right )
   float       getYAccel();                 // get y acceleration ( in Gs , positive body forward ) 
   float       getZAccel();                 // get z acceleration ( in Gs , positive body down )
   float       getAccelMag();               // get the magnitude of the accleration ( in Gs )
   float       getRoll();                   // get roll rotation  ( in degress , positive CW on X+ axis )
   float       getPitch();                  // get pitch rotation ( in degrees , positive CW on Y+ axis )
   
   bool        getZButton();                // get Z button status ( true = pressed )
   bool        getCButton();                // get C button status ( true = pressed )


  private:
  
    uint8_t    readBuffer [ NCDataSize ];   // raw I2C data buffer
    NCData     curSample;                   // most recent sampled datda
  
};


// global singleton class

extern NunchuckInterface Nunchuck;


// useful for debugging

void printfloat ( float val     ,           // Serial.print() a floating point value
                  int intWidth  ,           // with specified integer with ( up to 6 )
                  int fracWidth );          // and fractional parts ( up to 6 )


#endif // _NUNCHUCK_H_
