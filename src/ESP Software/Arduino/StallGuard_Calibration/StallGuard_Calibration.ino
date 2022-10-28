#include <Arduino.h>
#include <TMC2209.h>

HardwareSerial & serial_stream = Serial2;

const long SERIAL_BAUD_RATE = 115200;
const uint8_t RUN_CURRENT_PERCENT = 100;
const int32_t VELOCITY = 100000;
const uint8_t STALL_GUARD_THRESHOLD = 50;

TMC2209 stepper_driver1;
TMC2209 stepper_driver2;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  
  stepper_driver1.setup(serial_stream, SERIAL_BAUD_RATE);
  stepper_driver1.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver1.setStallGuardThreshold(STALL_GUARD_THRESHOLD);
  stepper_driver1.enable();

  stepper_driver2.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_1);
  stepper_driver2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver2.setStallGuardThreshold(STALL_GUARD_THRESHOLD);
  stepper_driver2.enable();

  calibrateXY(50);
}

void loop() {
  Serial.println("done");
}

void calibrateXY(unsigned int th) {
  while(1) {
    // Go down
    stepper_driver1.moveAtVelocity(-VELOCITY);
    stepper_driver2.moveAtVelocity(VELOCITY);
    Serial.print(stepper_driver1.getStallGuardResult());
    Serial.print("\t");
    Serial.println(stepper_driver1.getStallGuardResult());
    if(stepper_driver1.getStallGuardResult() < th || stepper_driver2.getStallGuardResult() < th) {
      stepper_driver1.moveAtVelocity(0);
      stepper_driver2.moveAtVelocity(0);
      delay(1000);
      break;
    }
  }

  while(1) {
    // Go left
    stepper_driver1.moveAtVelocity(-VELOCITY);
    stepper_driver2.moveAtVelocity(-VELOCITY);
    Serial.print(stepper_driver2.getStallGuardResult());
    Serial.print("\t");
    Serial.println(stepper_driver2.getStallGuardResult());
    if(stepper_driver1.getStallGuardResult() < th || stepper_driver2.getStallGuardResult() < th) {
      stepper_driver1.moveAtVelocity(0);
      stepper_driver2.moveAtVelocity(0);
      delay(1000);
      break;
    }
  }  
}
