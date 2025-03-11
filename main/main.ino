#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "lora_module.h"  // Обновленное имя модуля
#include "led.h"
#include "statistics.h"
#include "tasks.h"

void setup() {
    Serial.begin(115200);
    
    // Инициализация LED
    setupLed();
    
    // Инициализация LoRa
    if (!setupLoRa()) {
        Serial.println("LoRa init failed permanently. Blinking indefinitely...");
        while (true) {
            blinkLED(1, 200);
        }
    }
    Serial.println("LoRa started successfully!");
    blinkLED(3, 100);
    
    // Создание задач
    loraMutex = xSemaphoreCreateMutex();
    createTasks();
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}