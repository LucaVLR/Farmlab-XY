#include <TMC2209.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define ssid "bletchley"
#define password "laptop!internet"
#define mqtt_server "10.150.195.88"

#define upTopic "/MCU/UP"
#define downTopic "/MCU/DOWN"
#define leftTopic "/MCU/LEFT"
#define rightTopic "/MCU/RIGHT"
#define actionsTopic "/MCU/ACTIONS"
#define autoRouteTopic "/MCU/AUTOROUTE"

#define SERIAL_BAUD_RATE 115200
#define RUN_VELOCITY 2000
#define RUN_CURRENT_PERCENT 100
#define STALL_GUARD_THRESHOLD 100

WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial & serial_stream = Serial2;

bool startCalibration = false;
bool startAutoRoute = false;
bool takePicture = false;
bool stopMotors = false;

String cords;

TMC2209 stepper_driver;
TMC2209 stepper_driver2;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.setStallGuardThreshold(STALL_GUARD_THRESHOLD);
  stepper_driver.enable();
  stepper_driver.write(0x00, 385);
  stepper_driver.setMicrostepsPerStep(8);

  stepper_driver2.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_1);
  stepper_driver2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver2.setStallGuardThreshold(STALL_GUARD_THRESHOLD);
  stepper_driver2.enable();
  stepper_driver2.write(0x00, 385);
  stepper_driver2.setMicrostepsPerStep(8);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void calibrateXY(unsigned int th) {
  for(byte i = 0; i < 2; i++) {
    stepper_driver.moveAtVelocity(RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(RUN_VELOCITY);
    delay(10);
    
    while(1) {
      if(stepper_driver.getStallGuardResult() > th || stepper_driver2.getStallGuardResult() > th) {
        stepper_driver.moveAtVelocity(0);
        stepper_driver2.moveAtVelocity(0);
        delay(250);
        break;
      }
    }
  
    stepper_driver.moveAtVelocity(RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(-RUN_VELOCITY);
    delay(10);
    
    while(1) {
      if(stepper_driver.getStallGuardResult() > th || stepper_driver2.getStallGuardResult() > th) {
        stepper_driver.moveAtVelocity(0);
        stepper_driver2.moveAtVelocity(0);
        delay(250);
        break;
      }
    }    
  }

  client.publish("/MCU/CALIBRATION", "done");
}

void autoRoute(float x, float y) {
  if(x >= 0.0) {
    stepper_driver.moveAtVelocity(-RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(-RUN_VELOCITY);
    delay(x*100);
  }
  else {
    stepper_driver.moveAtVelocity(RUN_VELOCITY);
    stepper_driver2.moveAtVelocity(RUN_VELOCITY);
    delay((-x)*100);
  }

  stepper_driver.moveAtVelocity(-RUN_VELOCITY);
  stepper_driver2.moveAtVelocity(RUN_VELOCITY);
  delay(y*100);
  
  stepper_driver.moveAtVelocity(0);
  stepper_driver2.moveAtVelocity(0);
  delay(1000);
  startCalibration = true;
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String message = "";
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();
  
  if(String(topic) == leftTopic) {
    stepper_driver.moveAtVelocity(message.toInt());
    stepper_driver2.moveAtVelocity(message.toInt());
  }
  else if (String(topic) == rightTopic) {
    stepper_driver.moveAtVelocity(-message.toInt());
    stepper_driver2.moveAtVelocity(-message.toInt());
  }
  else if (String(topic) == upTopic) {
    stepper_driver.moveAtVelocity(-message.toInt());
    stepper_driver2.moveAtVelocity(message.toInt());
  }
  else if (String(topic) == downTopic) {
    stepper_driver.moveAtVelocity(message.toInt());
    stepper_driver2.moveAtVelocity(-message.toInt());
  }
  else if(String(topic) == actionsTopic) {
    if(message == "calibrate") {
      startCalibration = true;
    }
    else if(message == "picture") {
      takePicture = true;
    }
    else if(message == "stop") {
      stopMotors = true;
    }
  }
  else if(String(topic) == autoRouteTopic) {
    cords = message;
    startAutoRoute = true;
  }
  else {
    Serial.println("Unknown topic...");
    return;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(upTopic);
      client.subscribe(downTopic);
      client.subscribe(leftTopic);
      client.subscribe(rightTopic);
      client.subscribe(actionsTopic);
      client.subscribe(autoRouteTopic);
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float parseNextCords() {
  char strX[10];
  byte x = 0;

  do {
    strX[x] = cords[x];
    x++;
  } while((cords[x] != ',') & (x < cords.length()));

  strX[x] = '\0';
  cords.remove(0, x + 1);

  return atof(strX);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(startCalibration) {
    calibrateXY(300);
    startCalibration = false;
  }

  if(startAutoRoute) {
  float x, y;
    if(cords.length() > 20) {
      for(byte i = 0; i < 9; i++)
        autoRoute(parseNextCords(), parseNextCords());

      startAutoRoute = false;
      cords = "";
    }
    else {
      autoRoute(parseNextCords(), parseNextCords());
      startAutoRoute = false;
      cords = "";
    }
  }

  if(takePicture) {
    takePicture = false;
  }

  if(stopMotors) {
    stepper_driver.moveAtVelocity(0);
    stepper_driver2.moveAtVelocity(0);
    stopMotors = false;
  }
}
