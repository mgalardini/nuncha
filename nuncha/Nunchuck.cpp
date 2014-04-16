// ------------------------------------------------------------------------------------------------------
// Nunchuck - code the read the intpus from the Nintendo Wii NunChuck
// ------------------------------------------------------------------------------------------------------
//
// The Ninteno Wii nunchuck is a combination of 2 axis-joystick, 2 pushbuttons, and 3 axis acclerometer
// all sensed and interfaced via the I2C interface. This code reads the sensors and converts into real
// world data. This code was developed by Mike Engeldinger ( http://www.engeldinger.com ) and is in large
// part based on the WiiChuck library class found on the Arduino playground: 
//
//  http://arduino.cc/playground/Main/WiiChuckClass
//
// You are free to use this code for under the Creative Commons Arttribution-ShareAlike license, which 
// allows you to modify and redistribute the code as long as you credit the original authors as follows:
//
//  Original Authors:
//
//    Mike Engeldinger ( http://www.engeldinger.com )
//    Tim Hirzel       ( http://www.growdown.com )
//    Tod E. Kurt      ( http://todbot.com/blog/ )
//    Windmeadow Labs  ( http://www.windmeadow.com/node/42 ) 
//
// ------------------------------------------------------------------------------------------------------
//
// Connecting the Nunchuck to the Arduino
//
// To connect the nunchuck to the Arduino, you will need to convert the connector to bare wire. This can
// be done non-destructively using generic 3rd party controllers such as the NYKO Kama controller, which
// has screws holding the connector plug together. The connection requires 4 wires - the color coding
// below refers only to the Kama controller; other controllers may have different colored wires.
//
//  signal           Nunchuck            Arduino Uno
//  ------------     -------------       ---------------------------------
//  Power            red                 5V  ( Power Header Pin 5 )
//  Ground           black               GND ( Power Header Pin 6 or 7 )
//  SCL (Clock)      green               A5  ( AD Header Pin 6 )
//  SCA (Data)       yellow              A4  ( AD Header Pin 5 )
//
//
// ------------------------------------------------------------------------------------------------------
//
//  About the Accelerometer
//
// The accllerometer in the nunchuck is an ST Microelectronics LIS3L02A  MEMS linear accerometer that is 
// capable of 10 bit resolution over a range from zero 2 gs. The sensor is aligned on the nunchuck with
// it axes pointing X+ right, Y+ forward,  and +Z down. This is essentially mounted upside down w/r to
// the orientation in the LIS3L02A datasheet. When the sensor is motionless and level, the Z axis reports a 
// positive value of 1g ( earth gravity ). As the sensor rotates, this 1g acceleration vector rotates through 
// the device on the corresponding axes. This can be used to detect pitch and roll of the sensor as long as 
// the device is not in free-fall. The sensor can be also used to determine acceleration in body coordinates.
// To read linear sensor accleration in world coordinates you need to transform the values through a body
// to world rotation.
//
// In an ideal situation the output the sensor would use a bias of approximately 1/2 the 10 bit range (512). 
// Given the +/- 2g range limit, the output of a 1g vector in digital format would be 1/2 of that swing (256). 
// In practice however, the sensor has some variation. This code does allow you specify the bias and range of
// each axis, but uses typical values found from emperical testing. To obtain the most accuracy, you should
// calibrate these constants to your own device using the raw data print function found here. 
//
// The values obtained from the test device calibration ( 1g ) :
//
//            |  measured    |    computed    |           variance
//            |  min   max   |  bias    gain  |      bias          gain
//
//  X Axis    |  215 - 763   |  489     274   |   -23 (-2.2%)   +18 (+7.1%)
//  Y Axis    |  247 - 799   |  523     276   |   +11 (+1.1%)   +20 (+7.8%)
//  Z Axis    |  264 - 747   |  505     242   |    +7 (+0.7%)   -13 (-5.1%)
//
//  Average                  |  506     264   |    -2 (-0.1%)     8 ( +3.3%)
//                              ___     ___
//
//  Magnitude |                         458   |     ideal 442       (+3.6%)
//                                      ___
//
// ------------------------------------------------------------------------------------------------------
//
// About the stick
//
// The analog joystick is a dual axis shaft encoder capable of 8 bit range. In an ideal situation the
// output would have bias of 127, and a range of 127. Due the mechanical spring centering mechanism it
// is likey to have some bias error. To obtain themost accuracy, you should calibrate your stick against
// these values.
//
// The values obtained from the test device were:
//
//            |  measured       |       variance
//            |  min   max ctr  |  bias           gain
//
//  X Axis    |    0 - 255  128 |  +1 (+0.8%)   0 (0%)
//  Y Axis    |    0 - 255  125 |  -2 (+1.5%)   0 (0%)
//
//
// ------------------------------------------------------------------------------------------------------
//


#include "Arduino.h"                       // Arduino common function and defintions
#include "Nunchuck.h"                      // my own header file
#include "Wire.h"                          // I2C Arduino library

#define NCDevID  0x52                      // I2C 7 bit bus address [ 1010010 ]

#define AccelRange   264.0f                // accelerometer data range
#define AccelBias    506.0f                // accelerometer zero offset
#define Accel1GMag   458.0f                // accelerometer 1g magnitude

#define StickRange   127.0f                // analog stick data range
#define StickBias    127.0f                // analog stick zero offset

NunchuckInterface Nunchuck;                // interface class



// ------------------------------------------------------------------------------------------------------
// begin - intitialize the conection with the NunChuck
// ------------------------------------------------------------------------------------------------------

uint8_t NunchuckInterface::begin()
{
  uint8_t wireErr = 0;
  int byteCnt;
    
    
  // start the I2C library
  
  Wire.begin ();
  
            
  // send crypto key enable command ( to allow 3rd party controllers to work )

  Wire.beginTransmission ( NCDevID );
  Wire.write ( (uint8_t) 0xF0 );
  Wire.write ( (uint8_t) 0xAA );
  wireErr = Wire.endTransmission();
  if ( wireErr ) return ( wireErr );


  // write crypto key ( two 6 block writes, one 4 block write )
  
  delay ( 1 );
  Wire.beginTransmission( NCDevID );
  Wire.write ( (uint8_t) 0x40 );
  for ( byteCnt = 0; byteCnt < 6; byteCnt++ ) Wire.write ( (uint8_t) 0x00 );
  wireErr = Wire.endTransmission();
  if ( wireErr ) return ( wireErr );
    
  delay ( 1 );
  Wire.beginTransmission( NCDevID );
  Wire.write ( (uint8_t) 0x40 );
  for ( byteCnt = 6; byteCnt < 12; byteCnt++ ) Wire.write ( (uint8_t) 0x00 );
  wireErr = Wire.endTransmission();
  if ( wireErr ) return ( wireErr ); 
    
  delay ( 1 );
  Wire.beginTransmission( NCDevID );
  Wire.write ( (uint8_t) 0x40 );
  for ( byteCnt = 12; byteCnt < 16; byteCnt++ ) Wire.write ( (uint8_t) 0x00 );
  wireErr = Wire.endTransmission();
  if ( wireErr ) return ( wireErr );   
   
   
  // send a zero to the device to resample and store in register for next poll
   
  Wire.beginTransmission ( NCDevID );
  Wire.write ( ( uint8_t ) 0x00 );
  wireErr = Wire.endTransmission ();
  if ( wireErr ) return ( wireErr );
  delay ( 1 );
   
   return ( 0 );
}


// ------------------------------------------------------------------------------------------------------
// update - obtain the current sensor values of the Nunchuck
// ------------------------------------------------------------------------------------------------------

uint8_t NunchuckInterface::update()
{
  uint8_t wireErr = 0;
  
  
  // request data from device
  
  uint8_t numBytesRead = Wire.requestFrom ( NCDevID, NCDataSize );
  if ( numBytesRead != NCDataSize ) return ( 255 );

  
  
  // read the data and wait until all bytes have been read in
  // note that raw data must be XOR'ed by 0x17 + 0x17
  
  for ( int bufCnt = 0; bufCnt < NCDataSize; bufCnt++ )
  {
     uint8_t rawByte       = Wire.read();
     readBuffer [ bufCnt ] = ( rawByte ^ 0x17) + 0x17;
   }
  
   
   // convert the raw data in usable values
   
   curSample . xStick  =   readBuffer [ NCStickX ];
   curSample . yStick  =   readBuffer [ NCStickY ];
   curSample . xAccel  = ( readBuffer [ NCAccelX ] << 2 ) | ( ( readBuffer [ NCButtons ] >> 2 ) & 0x03 );
   curSample . yAccel  = ( readBuffer [ NCAccelY ] << 2 ) | ( ( readBuffer [ NCButtons ] >> 4 ) & 0x03 );
   curSample . zAccel  = ( readBuffer [ NCAccelZ ] << 2 ) | ( ( readBuffer [ NCButtons ] >> 6 ) & 0x03 );
   curSample . zButton = ( ( readBuffer [ NCButtons ] & 0x01 ) ? false : true );
   curSample . cButton = ( ( readBuffer [ NCButtons ] & 0x02 ) ? false : true );
   
   
   // convert the anlog stick into unit values
   
   curSample . xSReal = (float) ( curSample . xStick - StickBias ) / (float) StickRange;
   curSample . ySReal = (float) ( curSample . yStick - StickBias ) / (float) StickRange;
      
   
   // convert into accleration into g units
   
   curSample . xAReal = (float) ( curSample . xAccel - AccelBias ) / (float) AccelRange;
   curSample . yAReal = (float) ( curSample . yAccel - AccelBias ) / (float) AccelRange;
   curSample . zAReal = (float) ( curSample . zAccel - AccelBias ) / (float) AccelRange;
  
   
   // compute magnitude of g force vector
   
   curSample . rAReal = sqrt (   curSample . xAReal * curSample . xAReal 
                               + curSample . yAReal * curSample . yAReal
                               + curSample . zAReal * curSample . zAReal );

   
   // compute roll angle ( angle from X axis of G vector projected into X/Z plane )
   
   curSample . rollReal = 57.2957795f * atan2 ( curSample . xAReal , curSample . zAReal );
   
   
   // compute pitch angle ( angle from XY plane of G vector )
     
   if ( !curSample . rAReal ) curSample.pitchReal = 0.0f;
   else
     curSample . pitchReal = 57.2957795f * acos ( curSample . yAReal / curSample . rAReal );
   
   
   // send a zero to the device to resample and store in register for next poll
   
   Wire.beginTransmission ( NCDevID );
   Wire.write ( ( uint8_t ) 0x00 );
   wireErr = Wire.endTransmission ();
   if ( wireErr ) return ( wireErr );
   
   return ( 0 );  // no error
}


// ------------------------------------------------------------------------------------------------------
// getRaw -- get the raw data from the device
// ------------------------------------------------------------------------------------------------------

NCData * NunchuckInterface::getRaw ()
{
  return ( & curSample );
}

// ------------------------------------------------------------------------------------------------------
// get -- get the real world data from the device
// ------------------------------------------------------------------------------------------------------

float NunchuckInterface::getXStick()   { return ( curSample . xSReal ); }
float NunchuckInterface::getYStick()   { return ( curSample . ySReal ); }
float NunchuckInterface::getXAccel()   { return ( curSample . xAReal ); }
float NunchuckInterface::getYAccel()   { return ( curSample . yAReal ); }
float NunchuckInterface::getZAccel()   { return ( curSample . zAReal ); }
float NunchuckInterface::getAccelMag() { return ( curSample . rAReal ); }
float NunchuckInterface::getRoll()     { return ( curSample . rollReal ); }
float NunchuckInterface::getPitch()    { return ( curSample . pitchReal ); }

bool  NunchuckInterface::getZButton()  { return ( curSample . zButton ); }
bool  NunchuckInterface::getCButton()  { return ( curSample . cButton ); }


// ------------------------------------------------------------------------------------------------------
// printRaw - print the raw data to the serial monitor
// ------------------------------------------------------------------------------------------------------

void NunchuckInterface::printRaw()
{
  char strBuf[120];
  sprintf ( strBuf , "Z:%.1d\t C:%.1d\t X:%+.3d\t Y:%+.3d\t XA:%+.4d\t YA:%+.4d\t ZA:%+.4d" ,
                     (int) curSample . zButton , (int) curSample . cButton ,
                     (int) curSample . xStick  , (int) curSample . yStick  ,
                     (int) curSample . xAccel  , (int) curSample . yAccel  , (int) curSample . zAccel );
                     
  Serial.print ( strBuf );
}

// ------------------------------------------------------------------------------------------------------
// printRaw - print the real world data to the serial monitor
// ------------------------------------------------------------------------------------------------------

void NunchuckInterface::printReal()
{
  // convert to printable data 
  
  Serial.print ( "XG:"   ); printfloat ( curSample . xAReal    , 1 , 3  );
  Serial.print ( "\tYG:" ); printfloat ( curSample . yAReal    , 1 , 3  );
  Serial.print ( "\tZG:" ); printfloat ( curSample . zAReal    , 1 , 3  );
  Serial.print ( "\tPD:" ); printfloat ( curSample . pitchReal , 3 , 1  );
  Serial.print ( "\tRD:" ); printfloat ( curSample . rollReal  , 3 , 1  );
  
}

// ------------------------------------------------------------------------------------------------------
// formatFloat -- format a floating point number into decimal integer and fraction
// ------------------------------------------------------------------------------------------------------


void printfloat ( float val , int intWidth , int fracWidth )
{
  bool negative = false;
  
  // check for sign
  
  if ( val < 0 ) { val = -val; negative = true; }
  
    
  // compute scaling factor for fraction conversion ( up to 6 digits )
  
  long scalar;
  switch ( fracWidth )
  {
    default : scalar =       1; break;
    case 1  : scalar =      10; break;
    case 2  : scalar =     100; break;
    case 3  : scalar =    1000; break;
    case 4  : scalar =   10000; break;
    case 5  : scalar =  100000; break;
    case 6  : scalar = 1000000; break;
  }
  
  // compute integer and fractional parts
  
  long trueInt    = (long) val;
  long scaledInt  = (long) ( val * (float) scalar );
  long scaledFrac = scaledInt - ( trueInt * scalar );

  
  
  // build a format string
  
  char formatStr[11];

  formatStr[0] = ( negative ? '-' : '+' );
  formatStr[1] = '%';
  formatStr[2] = '.'; 
  formatStr[3] = '0' + (char) intWidth;
  formatStr[4] = 'd';
  formatStr[5] = '.';
  formatStr[6] = '%';
  formatStr[7] = '.';
  formatStr[8] = '0' + (char) fracWidth;
  formatStr[9] = 'd';
  formatStr[10] = 0x0;
  
  
  
  // print the string into buffer
  
  char outputStr[15];
  sprintf ( outputStr , formatStr , (int) trueInt , (int) scaledFrac );
  
  // send the string to the serial interface
  
  Serial.print ( outputStr );
 
}
 

