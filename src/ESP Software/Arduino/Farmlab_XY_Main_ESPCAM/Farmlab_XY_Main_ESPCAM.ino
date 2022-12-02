#include <TMC2209.h>

#include <WiFi.h>
#include <PubSubClient.h>

char* ssid = "bletchley";
char* password = "laptop!internet";
char* mqtt_server = "10.150.195.88";

#define upTopic "/MCU/UP"
#define downTopic "/MCU/DOWN"
#define leftTopic "/MCU/LEFT"
#define rightTopic "/MCU/RIGHT"
#define actionsTopic "/MCU/ACTIONS"

#define ENABLE 14

WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial & serial_stream = Serial;

const long SERIAL_BAUD_RATE = 115200;
const int32_t RUN_VELOCITY = 100000;
const uint8_t RUN_CURRENT_PERCENT = 100;
const uint8_t STALL_GUARD_THRESHOLD = 100;

bool startCalibration = false;
bool takePicture = false;

TMC2209 stepper_driver;
TMC2209 stepper_driver2;

void setup() {
  Serial.begin(115200);

  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.setStallGuardThreshold(STALL_GUARD_THRESHOLD);
  stepper_driver.enable();

  stepper_driver2.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_1);
  stepper_driver2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver2.setStallGuardThreshold(STALL_GUARD_THRESHOLD);
  stepper_driver2.enable();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void calibrateXY(unsigned int th) {
  stepper_driver.moveAtVelocity(-RUN_VELOCITY);
  stepper_driver2.moveAtVelocity(-RUN_VELOCITY);
  delay(10);
  
  while(1) {
    if(stepper_driver.getStallGuardResult() < th || stepper_driver2.getStallGuardResult() < th) {
      stepper_driver.moveAtVelocity(0);
      stepper_driver2.moveAtVelocity(0);
      delay(250);
      break;
    }
  }

  stepper_driver.moveAtVelocity(-RUN_VELOCITY);
  stepper_driver2.moveAtVelocity(RUN_VELOCITY);
  delay(10);
  
  while(1) {
    if(stepper_driver.getStallGuardResult() < th || stepper_driver2.getStallGuardResult() < th) {
      stepper_driver.moveAtVelocity(0);
      stepper_driver2.moveAtVelocity(0);
      delay(250);
      break;
    }
  }
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if(String(topic) == leftTopic) {
    stepper_driver.moveAtVelocity(-message.toInt());
    stepper_driver2.moveAtVelocity(-message.toInt());
  }
  else if (String(topic) == rightTopic) {
    stepper_driver.moveAtVelocity(message.toInt());
    stepper_driver2.moveAtVelocity(message.toInt());
  }
  else if (String(topic) == upTopic) {
    stepper_driver.moveAtVelocity(message.toInt());
    stepper_driver2.moveAtVelocity(-message.toInt());
  }
  else if (String(topic) == downTopic) {
    stepper_driver.moveAtVelocity(-message.toInt());
    stepper_driver2.moveAtVelocity(message.toInt());
  }
  else if(String(topic) == actionsTopic) {
    if(message == "calibrate") {
      startCalibration = true;
    }
    else if(message = "picture") {
      takePicture = true;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe(upTopic);
      client.subscribe(downTopic);
      client.subscribe(leftTopic);
      client.subscribe(rightTopic);
      client.subscribe(actionsTopic);
    } 
    else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(startCalibration) {
    calibrateXY(50);
    startCalibration = false;
  }

  if(takePicture) {
    takePicture = false;
  }
}
