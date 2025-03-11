#ifndef LED_H
#define LED_H

#include "config.h"

// Инициализация светодиода/NeoPixel
void setupLed();

// Функция мигания светодиодом с указанным цветом
void blinkLED(int times, int delayTime, uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255);

#endif // LED_H