#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
// Не включаем LoRa.h здесь, чтобы избежать конфликта
#include <Adafruit_NeoPixel.h>

// LoRa configuration
#define LORA_FREQUENCY    433E6
#define LORA_SPREADING    12
#define LORA_BANDWIDTH    31.25E3  
#define LORA_CODING_RATE  8
#define LORA_MAX_ATTEMPTS 5
#define LORA_TX_POWER     10

// Общие параметры
#define ALPHA 0.2  // Коэффициент сглаживания EWMA

// Настройка пинов для разных плат
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  #define SS          10
  #define RST         14
  #define DIO0        9
  #define LED_PIN     48
  #define NUM_LEDS    1
#elif defined(CONFIG_IDF_TARGET_ESP32)
  #define SS          15
  #define RST         14
  #define DIO0        4
  #define LED_BUILTIN 2
#else
  #error "Unsupported ESP32 variant"
#endif

// Глобальные переменные, используемые в разных модулях
extern SemaphoreHandle_t loraMutex;

#if defined(CONFIG_IDF_TARGET_ESP32S3)
extern Adafruit_NeoPixel strip;
#endif

#endif // CONFIG_H