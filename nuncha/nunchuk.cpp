#include <Arduino.h>
#include <Wire.h>
#include "nunchuk.h"
#define NUNCHUK_DEVICE_ID 0x52

void Nunchuk::initialize() {
  Wire.begin(); // <label id="code.nunchuk.master_joins"/>
  Wire.beginTransmission(NUNCHUK_DEVICE_ID);
  Wire.write((byte)0x40);
  Wire.write((byte)0x00);
  Wire.endTransmission();
  update();
}

bool Nunchuk::update() {
  //delay(1);
  Wire.requestFrom(NUNCHUK_DEVICE_ID, NUNCHUK_BUFFER_SIZE);
  int byte_counter = 0;
  while (Wire.available() && byte_counter < NUNCHUK_BUFFER_SIZE)
    _buffer[byte_counter++] = decode_byte(Wire.read());
  request_data();
  return byte_counter == NUNCHUK_BUFFER_SIZE;
}

void Nunchuk::request_data() {
  Wire.beginTransmission(NUNCHUK_DEVICE_ID);
  Wire.write((byte)0x00);
  Wire.endTransmission();
}

char Nunchuk::decode_byte(const char b) {
  return (b ^ 0x17) + 0x17;
}

