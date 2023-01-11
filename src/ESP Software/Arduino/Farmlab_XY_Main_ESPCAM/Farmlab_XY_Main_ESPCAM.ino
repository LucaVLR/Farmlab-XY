#include "Arduino.h"
#include <TMC2209.h>
#include "driver/rtc_io.h"
#include "esp_camera.h"
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
#define pictureTopic "/MCU/FOTOS"

#define ENABLE 14

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Base64 alphabet + padding char
#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define PAD '='

// pointer for the camera frame buffer
camera_fb_t * fb = NULL;

WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial & serial_stream = Serial;

#define SERIAL_BAUD_RATE 115200
#define RUN_VELOCITY 2000
#define RUN_CURRENT_PERCENT 100

bool startCalibration = false;
bool startAutoRoute = false;
bool takePicture = false;
bool stopMotors = false;

String cords;

TMC2209 stepper_driver;
const TMC2209::SerialAddress SERIAL_ADDRESS_0 = TMC2209::SERIAL_ADDRESS_0;
TMC2209 stepper_driver2;
const TMC2209::SerialAddress SERIAL_ADDRESS_1 = TMC2209::SERIAL_ADDRESS_1;

void setup() {
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  
  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE);
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enable();
  stepper_driver.write(0x00, 385);
  stepper_driver.setMicrostepsPerStep(8);

  stepper_driver2.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_1);
  stepper_driver2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver2.enable();
  stepper_driver2.write(0x00, 385);
  stepper_driver2.setMicrostepsPerStep(8);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return;
  }

  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);
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
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe(upTopic);
      client.subscribe(downTopic);
      client.subscribe(leftTopic);
      client.subscribe(rightTopic);
      client.subscribe(actionsTopic);
      client.subscribe(autoRouteTopic);
    } 
    else {
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
      for(byte i = 0; i < 9; i++) {
        autoRoute(parseNextCords(), parseNextCords());
        takeEncodePicture();
      }
      
      startAutoRoute = false;
      cords = "";
    }
    else {
      autoRoute(parseNextCords(), parseNextCords());
      takeEncodePicture();
      startAutoRoute = false;
      cords = "";
    }
  }

  if(takePicture) {
    takeEncodePicture();
    takePicture = false;
  }

  if(stopMotors) {
    stepper_driver.moveAtVelocity(0);
    stepper_driver2.moveAtVelocity(0);
    stopMotors = false;
  }
}

void takeEncodePicture() {
  digitalWrite(4, HIGH);    // Enable camera flash LED
  fb = esp_camera_fb_get(); // Take picture
  if(!fb) {
    return;
  }
  digitalWrite(4, LOW); // Turn off camera flash LED

  esp_camera_fb_return(fb);
  
  byte imgdata[3];    // holds 3 bytes of frame buffer data for base64 conversion
  char based[4];      // holds the resulting 4 chars of encoded data
  byte index = 0;     // manages the index of the base64 alphabet, based on 6 bits of imgdata
  byte convint = 0;   // holds interrupted index when padding is required
  bool pad = false;   // tells if padding is required
  String pic_str = "";// stores the final base64 string.

  // This loops through the frame buffer, encoding it into a single base64 string
  for(size_t bufi = 0; bufi < fb->len; bufi+=3) {
    // Take the next 3 bytes out of the frame buffer, break and pad if no bytes are left
    for(byte i = 0; i < 3; i++) {
      imgdata[i] = fb->buf[bufi + i];
      if((bufi + i) == fb->len) {
        convint = i;
        pad = true;
        break;
      }
    }

    // add padding, standard in base64 this is done by adding trailing 0's to the remaining data and then encoding
    // encoded values that are 0 result in padding character '='
    if(pad) {
      switch(convint) {
        case 0:
          imgdata[1] = 0;
          imgdata[2] = 0;
          based[3] = '=';
          based[4] = '=';

          index = imgdata[0] & 0b11111100;
          index = index >> 2;
          based[0] = ALPHABET[index];
      
          index = ((imgdata[0] & 0b00000011) << 4) + ((imgdata[1] & 0b11110000) >> 4);
          based[1] = ALPHABET[index];
          break;
          
        case 1:
          imgdata[2] = 0;
          based[3] = '=';

          index = imgdata[0] & 0b11111100;
          index = index >> 2;
          based[0] = ALPHABET[index];
      
          index = ((imgdata[0] & 0b00000011) << 4) + ((imgdata[1] & 0b11110000) >> 4);
          based[1] = ALPHABET[index];
          
          index = ((imgdata[1] & 0b00001111) << 2) + ((imgdata[2] & 0b11000000) >> 6);
          based[2] = ALPHABET[index];
          break;
      }
    }

    // else continue encoding the 3 bytes into base64 chars
    // this is done by taking 6 bits and turning them into an index
    // this index then points to a char in a char array ALPHABET which contains the base64 alphabet
    // rinse and repeat until you get 4 chars
    else {
      index = imgdata[0] & 0b11111100;
      index = index >> 2;
      based[0] = ALPHABET[index];
  
      index = ((imgdata[0] & 0b00000011) << 4) + ((imgdata[1] & 0b11110000) >> 4);
      based[1] = ALPHABET[index];
      
      index = ((imgdata[1] & 0b00001111) << 2) + ((imgdata[2] & 0b11000000) >> 6);
      based[2] = ALPHABET[index];
  
      index = imgdata[2] & 0b00111111;
      based[3] = ALPHABET[index];
    }

    // add the encoded base64 chars to the full string
    for(byte i = 0; i < 4; i++)
      pic_str += based[i];
  }

  // PubSubClient has a standard send limit of 256 bytes
  // our base64 string can be about 30k bytes, meaning we have to expand accordingly
  // we also have to leave room for the message headers, about 128 bytes should suffice
  client.setBufferSize(pic_str.length()+128);

  // Ensure a proper connection before attempting to send data
  if(!client.connected()) {
    reconnect();
  }

  // Send the picture through MQTT
  // NOTE: not entirely stable in single picture testing, might need a better QoS or buffer size
  client.publish(pictureTopic, (char*)pic_str.c_str());

  pic_str = ""; // clear the string, hopefully this saves memory if ESP has decent garbage collection
}
