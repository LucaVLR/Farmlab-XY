#include <Arduino.h>
#include <TMC2209.h>

HardwareSerial & serial_stream = Serial2;

const long SERIAL_BAUD_RATE = 115200;
const int32_t RUN_VELOCITY = 20000;
const int32_t STOP_VELOCITY = 0;
const int RUN_DURATION = 2000;
const int STOP_DURATION = 1000;
const uint8_t RUN_CURRENT_PERCENT = 100;

// Instantiate TMC2209
TMC2209 stepper_driver;
bool invert_direction = false;

void setup()
{
  pinMode(5, OUTPUT);
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("setup:");
  delay(100);
  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE);
  
  Serial.println("\nenable:");
  delay(100);
  stepper_driver.enable();
  Serial.println("\nmoveAtVelocity:");
  delay(100);
  stepper_driver.moveAtVelocity(RUN_VELOCITY);
}

void loop()
{
  
}
