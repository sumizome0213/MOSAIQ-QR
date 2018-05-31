// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

#include <GxEPD.h>

#include <GxGDEW042T2/GxGDEW042T2.cpp>      // 4.2" b/w

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#include "qrcode.h"

#include "esp_deep_sleep.h"

//esp32
GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

#define SwitchPin 0

int count = 0;
unsigned long time;

void setup() {
  Serial.begin(115200); 
  Serial.println();
  Serial.println("setup");

  pinMode(SwitchPin, INPUT_PULLUP);

  display.init(115200); // enable diagnostic output on Serial

  Serial.println("setup done");

  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
  gpio_pullup_en(GPIO_NUM_0);    // use pullup on GPIO
  gpio_pulldown_dis(GPIO_NUM_0); // not use pulldown on GPIO

  esp_deep_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // 指定したGPIOがLowの時に起動

  drawQR("E-ink QR - " + String(count));
  count++;

  time = millis();

}

void loop() {
  if(digitalRead(SwitchPin) == LOW){
    drawQR("E-ink QR - " + String(count));
    count++;
    time = millis();
  }

  if(millis() - time > 10*1000) {
    Serial.println("deep sleep start");
    esp_deep_sleep_start(); // スリープモード実行
  }
}

void drawQR(String str) {
  char msg[100];
  str.toCharArray(msg,str.length()+1); 
  
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, msg);

  uint16_t pixel_size = 8;
  uint16_t box_x = (400-qrcode.size*pixel_size)/2;
  uint16_t box_y = (300-qrcode.size*pixel_size)/2;
  uint16_t box_w = qrcode.size*pixel_size;
  uint16_t box_h = qrcode.size*pixel_size;

  display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
       if(qrcode_getModule(&qrcode, x, y)){
         display.fillRect(box_x + pixel_size*x, box_y + pixel_size*y, pixel_size, pixel_size, GxEPD_BLACK);
       }
    }
  }

//  display.updateWindow(box_x, box_y, box_w, box_h, true);
  display.update();
  
}
