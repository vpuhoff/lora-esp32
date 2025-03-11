#include "led.h"

void setupLed() {
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
        strip.begin();
        strip.setBrightness(50);
        strip.show();
    #else
        pinMode(LED_BUILTIN, OUTPUT);
    #endif
}

void blinkLED(int times, int delayTime, uint8_t red, uint8_t green, uint8_t blue) {
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
        strip.setBrightness(50);
        strip.setPixelColor(0, strip.Color(red, green, blue));
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(delayTime));
        strip.clear();
        strip.show();
    #else
        for (int i = 0; i < times; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(delayTime));
            digitalWrite(LED_BUILTIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(delayTime));
        }
    #endif
}