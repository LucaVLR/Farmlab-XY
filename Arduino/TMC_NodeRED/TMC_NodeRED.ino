#include <TMC2209.h>

#include <WiFi.h>
#include <PubSubClient.h>

char* ssid = "Jelte Laptop";
char* password = "jelteniels";
char* mqtt_server = "mqtt.luytsm.be";

#define upTopic "/MCU/UP"
#define downTopic "/MCU/DOWN"
#define leftTopic "/MCU/LEFT"
#define rightTopic "/MCU/RIGHT"

WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial & serial_stream = Serial2;

const long SERIAL_BAUD_RATE = 115200;
const int32_t RUN_VELOCITY = 100000;
const uint8_t RUN_CURRENT_PERCENT = 10;
const uint8_t STALL_GUARD_THRESHOLD = 100;

TMC2209 stepper_driver;
TMC2209 stepper_driver2;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

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
  String aantalSteps;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    aantalSteps += (char)payload[i];
  }
  Serial.println();
  
  if(String(topic) == leftTopic) {
    stepper_driver.moveAtVelocity(aantalSteps.toInt());
    stepper_driver2.moveAtVelocity(-aantalSteps.toInt());
    Serial.println("Going " + aantalSteps + " to the left");
  }
  else if (String(topic) == rightTopic) {
    stepper_driver.moveAtVelocity(-aantalSteps.toInt());
    stepper_driver2.moveAtVelocity(aantalSteps.toInt());
    Serial.println("Going " + aantalSteps + " the the right");
  }
  else if (String(topic) == upTopic) {
    stepper_driver.moveAtVelocity(-aantalSteps.toInt());
    stepper_driver2.moveAtVelocity(-aantalSteps.toInt());
    Serial.println("Going " + aantalSteps + " upwards");
  }
  else if (String(topic) == downTopic) {
    stepper_driver.moveAtVelocity(aantalSteps.toInt());
    stepper_driver2.moveAtVelocity(aantalSteps.toInt());
    Serial.println("Going " + aantalSteps + " downwards");
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
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
