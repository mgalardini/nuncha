#include <Wire.h>
#include "nunchuk.h"
#include <MozziGuts.h>

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 64 // powers of 2 please

Nunchuk nunchuk;

const unsigned int BAUD_RATE = 19200;

// Stabilizzatore joystick

#define REST_POS 128
#define JOYSTICK_THRESHOLD 10
int volume = 1;

boolean joystick_x_rest(int joy_x){
  if(abs(joy_x - REST_POS) < JOYSTICK_THRESHOLD){
    return true;
  }
  return false;   
}

boolean joystick_y_rest(int joy_y){
  if(abs(joy_y - REST_POS) < JOYSTICK_THRESHOLD){
    return true;
  }
  return false;   
}

// Rilevatore di frustata

int prev_x = 0;
int prev_y = 0;
int prev_z = 0;
boolean whip_start = true;

//#define WHIP_THRESHOLD 40
//#define WHIP_COUNT_THRESHOLD 5
#define WHIP_THRESHOLD 55
#define WHIP_COUNT_THRESHOLD 5
#define NO_WHIP 0
#define WHIP_X_UP 1
#define WHIP_X_DOWN 2
#define WHIP_Y_UP 3
#define WHIP_Y_DOWN 4
#define WHIP_Z_UP 5
#define WHIP_Z_DOWN 6

// Whip counters
int whip_x_up = 0;
int whip_x_down = 0;
int whip_y_up = 0;
int whip_y_down = 0;
int whip_z_up = 0;
int whip_z_down = 0;

void reset_whip(){
  whip_x_up = 0;
  whip_x_down = 0;
  whip_y_up = 0;
  whip_y_down = 0;
  whip_z_up = 0;
  whip_z_down = 0;
}

int whip(int x, int y, int z){
  int delta_x = x - prev_x;
  int delta_y = y - prev_y;
  int delta_z = z - prev_z;
  
  if(abs(delta_x) > abs(delta_y) && abs(delta_x) > abs(delta_z) && abs(delta_x) > WHIP_THRESHOLD){
    if(delta_x > 0){
      whip_x_down += 1;
    }
    else{
      whip_x_up += 1;
    }
  }
  if(abs(delta_y) > abs(delta_x) && abs(delta_y) > abs(delta_z) && abs(delta_y) > WHIP_THRESHOLD){
    if(delta_y > 0){
      whip_y_down += 1;
    }
    else{
      whip_y_up += 1;
    }
  }
  if(abs(delta_z) > abs(delta_y) && abs(delta_z) > abs(delta_x) && abs(delta_z) > WHIP_THRESHOLD){
    if(delta_z > 0){
      whip_z_down += 1;
    }
    else{
      whip_z_up += 1;
    }
  }
  
  // Here put a check on the max value above the counters
  // Try with an array!
  if(whip_x_up > WHIP_COUNT_THRESHOLD && whip_x_up > whip_x_down){
    reset_whip();
    return WHIP_X_UP;
  }
  if(whip_x_down > WHIP_COUNT_THRESHOLD && whip_x_down > whip_x_up){
    reset_whip();
    return WHIP_X_DOWN;
  }
  if(whip_y_up > WHIP_COUNT_THRESHOLD && whip_y_up > whip_y_down){
    reset_whip();
    return WHIP_Y_UP;
  }
  if(whip_y_down > WHIP_COUNT_THRESHOLD && whip_y_down > whip_y_up){
    reset_whip();
    return WHIP_Y_DOWN;
  }
  if(whip_z_up > WHIP_COUNT_THRESHOLD && whip_z_up > whip_z_down){
    reset_whip();
    return WHIP_Z_UP;
  }
  if(whip_z_down > WHIP_COUNT_THRESHOLD && whip_z_down > whip_z_up){
    reset_whip();
    return WHIP_Z_DOWN;
  }
  
  return NO_WHIP;
}

// Anti_bouncing
#define PRESS_THRESHOLD 5

int c_presses = 0;
boolean c_pressed = false;
int prev_c_but = 0;
boolean c_swap = false;

boolean c_press(){
  if(c_presses > PRESS_THRESHOLD && c_swap){
    c_presses = 0;
    if(c_pressed){c_pressed = false;}
    else{c_pressed = true;}
    return true;
  }
  return false;
}

boolean is_c_pressed(){
  return c_pressed;
}

int z_presses = 0;
boolean z_pressed = false;
int prev_z_but = 0;
boolean z_swap = false;

boolean z_press(){
  if(z_presses > PRESS_THRESHOLD && z_swap){
    z_presses = 0;
    if(z_pressed){z_pressed = false;}
    else{z_pressed = true;}
    return true;
  }
  return false;
}

boolean is_z_pressed(){
  return z_pressed;
}

void setup(){
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)

  nunchuk.initialize();

  Serial.begin(BAUD_RATE);
  Serial.println("I wana ghana");
}

void updateControl(){
  if(nunchuk.update()){
/*  if (millis() % 1000 < 20){
  
 Serial.print(
             whip(nunchuk.x_acceleration(), nunchuk.y_acceleration(), nunchuk.z_acceleration())
             );
 Serial.print(" ");
 Serial.print(joystick_x_rest(nunchuk.joystick_x()));
 Serial.print(" ");
 Serial.println(joystick_y_rest(nunchuk.joystick_y()));
  }*/
    if(whip_start){
      prev_x = nunchuk.x_acceleration();
      prev_y = nunchuk.y_acceleration();
      prev_z = nunchuk.z_acceleration();
      whip_start = false;
    }
  
       Serial.println(
               whip(nunchuk.x_acceleration(), nunchuk.y_acceleration(), nunchuk.z_acceleration())
               );
  
    prev_x = nunchuk.x_acceleration();
    prev_y = nunchuk.y_acceleration();
    prev_z = nunchuk.z_acceleration();
    
    if(nunchuk.c_button() != prev_c_but){c_swap = true;}
    else{c_swap = false;}
    
    if(nunchuk.z_button() != prev_z_but){z_swap = true;}
    else{z_swap = false;}
    
    c_presses += nunchuk.c_button();
    z_presses += nunchuk.z_button();
    
    prev_c_but = nunchuk.c_button();
    prev_z_but = nunchuk.z_button();
  }
 /*Serial.print(
             whip(nunchuk.x_acceleration(), nunchuk.y_acceleration(), nunchuk.z_acceleration())
             );
 Serial.print(" ");            
 Serial.print(c_press());
 Serial.print(" ");
 Serial.print(is_c_pressed());
 Serial.print(" ");
 Serial.print(z_press());
 Serial.print(" ");
 Serial.print(is_z_pressed());
 Serial.print(" ");
 Serial.print(joystick_x_rest(nunchuk.joystick_x()));
 Serial.print(" ");
 Serial.println(joystick_y_rest(nunchuk.joystick_y()));*/
}

int updateAudio(){
  return 0;
}

void loop(){
  audioHook();
}
