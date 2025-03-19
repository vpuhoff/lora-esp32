#include "config.h"
#include <LoRa.h>

// Определение глобальных переменных
SemaphoreHandle_t spi_lock_mutex;

#if defined(CONFIG_IDF_TARGET_ESP32S3)
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif