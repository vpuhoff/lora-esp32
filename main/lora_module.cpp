#include "lora_module.h"
#include "led.h"

bool setupLoRa() {
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    logger.println("Initializing LoRa...");
    int attempts = 0;
    if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(10000))) {
        while (!LoRa.begin(LORA_FREQUENCY) && attempts < LORA_MAX_ATTEMPTS) {
            logger.println("LoRa init failed, retrying...");
            blinkLED(2, 200);
            vTaskDelay(pdMS_TO_TICKS(10000));
            attempts++;
        }
        xSemaphoreGive(spi_lock_mutex);
    }
    
    if (attempts == LORA_MAX_ATTEMPTS) {
        return false;
    }
    
    // Настройка параметров LoRa
    LoRa.setSpreadingFactor(LORA_SPREADING);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setTxPower(LORA_TX_POWER);
    
    return true;
} 

// Перезапуск LoRa-модуля
void restartLoRaModule() {
  Serial.println("Перезапуск LoRa-модуля...");
  // Выполняем аппаратный сброс через пин RESET
  digitalWrite(LORA_RST, LOW);
  delay(100);  // Короткая задержка для сброса
  digitalWrite(LORA_RST, HIGH);
  delay(100);  // Задержка для стабилизации
  // Переинициализируем модуль
  setupLoRa();
}