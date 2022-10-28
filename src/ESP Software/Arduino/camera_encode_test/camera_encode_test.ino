#include "esp_camera.h"
#include "Arduino.h"
#include "driver/rtc_io.h"
#include <WiFi.h>
#include <PubSubClient.h>

char* ssid = "bletchley";
char* password = "laptop!internet";
char* mqtt_server = "10.150.195.88";

#define topic "/FOTOS"

WiFiClient espClient;
PubSubClient client(espClient);

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

#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define PAD '='

camera_fb_t * fb = NULL;

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

void setup() {
  Serial.begin(115200);
  delay(3000);

  //setup_wifi();

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
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
  config.pixel_format = PIXFORMAT_JPEG;  // normaal PIXFORMAT_JPEG
  /*
  FRAMESIZE_UXGA (1600 x 1200)
  FRAMESIZE_QVGA (320 x 240)
  FRAMESIZE_CIF (352 x 288)
  FRAMESIZE_VGA (640 x 480)
  FRAMESIZE_SVGA (800 x 600)
  FRAMESIZE_XGA (1024 x 768)
  FRAMESIZE_SXGA (1280 x 1024)
  jpeg_quality = number between 0-63, lower value = higher quality
  */
  /*if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    // medium picture, medium size
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }*/
  config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //sensor_t * s = esp_camera_sensor_get();

  takeEncodePicture();
}

void takeEncodePicture() {
  Serial.println("Taking picture");
  pinMode(4, OUTPUT);
  delay(6000); // resolves issues with colour tints, don't know why ¯\_(ツ)_/¯
  digitalWrite(4, HIGH);
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  Serial.println(fb->len);
  digitalWrite(4, LOW);

  esp_camera_fb_return(fb);
    
  byte imgdata[3];
  char based[4];
  byte index = 0;
  byte convint = 0;
  bool pad = false;
  for(size_t bufi = 0; bufi < fb->len; bufi+=3) {
    for(byte i = 0; i < 3; i++) {
      imgdata[i] = fb->buf[bufi + i];
      if((bufi + i) == fb->len) {
        convint = i;
        pad = true;
        break;
      }
    }
    
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
    
    for(byte i = 0; i < 4; i++)
      Serial.print(based[i]);
  }
}

void loop() {
  
}
