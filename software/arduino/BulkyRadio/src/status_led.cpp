#include <Arduino.h>
#ifdef CONFIG_IDF_TARGET_ESP32S3
#include <Adafruit_NeoPixel.h>
#endif
#include "constants.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3
Adafruit_NeoPixel pixels(NEOPIXEL_NUM, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

void set_status(int value) {
  #ifdef CONFIG_IDF_TARGET_ESP32S3
  pixels.fill(value);
  pixels.show();
  #else
  digitalWrite(ONBOARD_LED, (value == 0 ? LOW : HIGH));
  #endif
}

void init_status() {
  #ifdef CONFIG_IDF_TARGET_ESP32S3
  pixels.begin();
  pixels.setBrightness(NEOPIXEL_LUM);
  set_status(STATUS_NONE);
  #else
  pinMode(ONBOARD_LED, OUTPUT);
  #endif
}