#include "tasks.h"
#include "led.h"
#include "statistics.h"
#include "plot-manager.h"
#include "lora-manager.h" 
#include "system-monitor.h"
#include "display-manager.h"
#include <WiFi.h>
#include <SettingsESPWS.h>
#include "esp_task_wdt.h"

// Объявление внешних переменных, используемых в задаче веб-интерфейса
extern SettingsESPWS sett;

void createTasks() {
    // LoRa-related tasks on Core 1
    xTaskCreatePinnedToCore(taskSendHello, "SendHello", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(taskReceive, "Receive", 4096, NULL, 3, NULL, 1);
    
    // Lower priority for monitoring tasks
    xTaskCreatePinnedToCore(taskMonitorStack, "StackMonitor", 4096, NULL, 1, NULL, 1);
    
    // UI and display tasks on Core 0 with appropriate priorities
    xTaskCreatePinnedToCore(taskWebInterface, "WebInterface", 16384, NULL, 2, NULL, 0);
    #if DISPLAY_ENABLED
    // Задача обновления дисплея только для ESP32
    xTaskCreatePinnedToCore(taskDisplayUpdate, "DisplayUpdate", 4096, NULL, 1, NULL, 0);
    #endif
}

void taskSendHello(void *parameter) {
    esp_task_wdt_add(NULL);
    while (true) {
        if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(4000))) {
            int currentPacketId = packetId++;
            
            uint32_t startTime = millis();
            LoRa.beginPacket();
            LoRa.print("HLO:");
            LoRa.print(currentPacketId); // Отправляем ID пакета
            LoRa.endPacket();
            uint32_t duration = millis() - startTime;
            Serial.printf("Hello packet %d sent, transmission time: %u ms\n", 
                         currentPacketId, duration);

            xSemaphoreGive(spi_lock_mutex);
            
            // Отмечаем, что пакет отправлен, но пока не подтвержден
            updateStats(false);
            blinkLED(3, 50, 255, 0, 0); // Красный
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(random(15000, 30000)));
    }
}

void taskReceive(void *parameter) {
    esp_task_wdt_add(NULL);
    for (;;) {
        if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(5000))) {
            int packetSize = LoRa.parsePacket();

            if (packetSize) {

                String incoming = "";
                while (LoRa.available()) {
                    //int Rssi = LoRa.packetRssi();
                    //Serial.printf("RSSI: %s\n", Rssi);
                    incoming += (char)LoRa.read();
                }
                incoming.trim();

                Serial.printf("Received: %s\n", incoming.c_str());

                if (incoming.startsWith("HLO:")) {
                    // Извлекаем ID пакета
                    int receivedId = incoming.substring(4).toInt();
                    
                    logger.println("Hello received! Sending ACK...");
                    uint32_t startTime = millis();
                    LoRa.beginPacket();
                    LoRa.print("ACK:");
                    LoRa.print(receivedId); // Отправляем ID пакета в ACK
                    LoRa.endPacket();
                    xSemaphoreGive(spi_lock_mutex);
                    uint32_t duration = millis() - startTime;
                    Serial.printf("ACK packet sent, transmission time: %u ms\n", duration);
                    blinkLED(2, 1000, 0, 255, 0); // Зелёный
                } else if (incoming.startsWith("ACK:")) {
                    xSemaphoreGive(spi_lock_mutex);
                    // Получили подтверждение
                    int ackId = incoming.substring(4).toInt();
                    Serial.printf("ACK received for packet %d!\n", ackId);

                    // Обновляем статистику только для этого пакета
                    updatePacketStatus(ackId, true);
                    
                    blinkLED(2, 1000, 0, 0, 255); // Синий
                } else {
                    xSemaphoreGive(spi_lock_mutex);
                }
            } else {
                xSemaphoreGive(spi_lock_mutex);
            }
        } else {
          logger.println("Failed to acquire mutex for receive!");
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Заменяем существующую функцию taskMonitorStack
void taskMonitorStack(void *parameter) {
    esp_task_wdt_add(NULL);
    // Создаем экземпляр SystemMonitor, если он еще не создан
    if (systemMonitor == nullptr) {
        systemMonitor = new SystemMonitor();
    }
    
    for (;;) {
        // Обновляем данные мониторинга
        systemMonitor->update();
        
        // Логируем базовую информацию о стеке
        // UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL);
        // Serial.printf("Free stack: %d bytes\n", freeStack);
        
        // Периодически логируем полную статистику (раз в минуту)
        static uint8_t fullLogCounter = 0;
        if (++fullLogCounter >= 6) {  // 6 * 10 секунд = 1 минута
            systemMonitor->logTasksStatistics();
            systemMonitor->logMemoryStatistics();
            fullLogCounter = 0;
        }
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10000));  // Проверка каждые 10 секунд
    }
}

// Задача для обработки веб-интерфейса
void taskWebInterface(void *parameter) {
    esp_task_wdt_add(NULL);
    for (;;) {
        sett.tick();
        
        // Обновление данных для графика каждые 500 мс
        // static uint32_t plotTimer = 0;
        // if (millis() - plotTimer >= 500) {
        //     plotTimer = millis();
        //     plotManager.updateData();
        // }
        
        // Периодическое обновление данных LoRa каждые 5000 мс
        static uint32_t loraTimer = 0;
        if (millis() - loraTimer >= 5000) {
            loraTimer = millis();
            loraManager->updateStats();
        }
        
        // Периодическое логирование состояния системы каждые 10000 мс
        static uint32_t logTimer = 0;
        if (millis() - logTimer >= 10000) {
            logTimer = millis();
            if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
                if (WiFi.status() == WL_CONNECTED) {
                    logger.println("WiFi подключен к " + WiFi.SSID() + ", сигнал: " + String(WiFi.RSSI()) + " dBm");
                }
            }
            logger.println("Свободная память: " + String(ESP.getFreeHeap()) + " байт");
        }
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10)); // Небольшая задержка для экономии ресурсов
    }
}

void taskDisplayUpdate(void *parameter) {
    esp_task_wdt_add(NULL);
    for (;;) {
        if (displayManager && displayManager->isEnabled()) {
            displayManager->tick();
            displayManager->updateCurrentPage();
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500)); // Обновление каждые 500 мс
    }
}