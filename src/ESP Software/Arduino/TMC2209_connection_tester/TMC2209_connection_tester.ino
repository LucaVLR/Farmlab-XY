#include <Arduino.h>
#include <TMC2209.h>

HardwareSerial & serial_stream = Serial2;

const long SERIAL_BAUD_RATE = 115200;
const uint8_t RUN_CURRENT_PERCENT = 100;

// Instantiate TMC2209
TMC2209 stepper_driver;

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial2.begin(SERIAL_BAUD_RATE);

  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_1);
  //stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enable();
}

void loop()
{
  if (stepper_driver.isSetupAndCommunicating())
  {
    Serial.println("connected");
  }
  else
  {
    Serial.println("not connected");
  }

  delay(200);
}
