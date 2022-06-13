#include <WiFi.h>
#include <PubSubClient.h>

char* ssid = "Jelte Laptop";
char* password = "jelteniels";
char* mqtt_server = "mqtt.luytsm.be";

#define upTopic "/MCU/UP"
#define downTopic "/MCU/DOWN"
#define leftTopic "/MCU/LEFT"
#define rightTopic "/MCU/RIGHT"

// CW == HIGH, CCW == LOW
#define STEP1 13
#define DIR1 12

#define STEP2 15
#define DIR2 14
 
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(STEP1, OUTPUT);
  pinMode(STEP2, OUTPUT);
  
  Serial.begin(115200);
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
    digitalWrite(DIR1, HIGH); // CW
    digitalWrite(DIR2, LOW);  // CCW
    Serial.println("Going " + aantalSteps + " to the left");
  }
  else if (String(topic) == rightTopic) {
    digitalWrite(DIR1, LOW);  // CCW
    digitalWrite(DIR2, HIGH); // CW
    Serial.println("Going " + aantalSteps + " the the right");
  }
  else if (String(topic) == upTopic) {
    digitalWrite(DIR1, LOW);  // CCW
    digitalWrite(DIR2, LOW);  // CCW
    Serial.println("Going " + aantalSteps + " upwards");
  }
  else if (String(topic) == downTopic) {
    digitalWrite(DIR1, HIGH); // CW
    digitalWrite(DIR2, HIGH); // CW
    Serial.println("Going " + aantalSteps + " downwards");
  }
  else {
    Serial.println("Unknown topic...");
    return;
  }

  for(long i = 0; i < aantalSteps.toInt(); i++) {
    digitalWrite(STEP1, HIGH);
    digitalWrite(STEP2, HIGH);
    delayMicroseconds(50);

    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    delayMicroseconds(50);
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
