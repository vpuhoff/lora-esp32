#include "tasks.h"
#include "led.h"
#include "statistics.h"
#include "plot-manager.h"
#include "lora-manager.h" 
#include "system-monitor.h"
#include "display-manager.h"
#include <WiFi.h>
#include <SettingsESPWS.h>

// Объявление внешних переменных, используемых в задаче веб-интерфейса
extern SettingsESPWS sett;

void createTasks() {
    xTaskCreatePinnedToCore(taskSendHello, "SendHello", 3072, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskReceive, "Receive", 3072, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskMonitorStack, "StackMonitor", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskWebInterface, "WebInterface", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(taskDisplayUpdate, "DisplayUpdate", 4096, NULL, 1, NULL, 0);
}

void taskSendHello(void *parameter) {
    while (true) {
        if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(4000))) {
            int currentPacketId = packetId++;
            
            uint32_t startTime = millis();
            LoRa.beginPacket();
            LoRa.print("HLO:");
            LoRa.print(currentPacketId); // Отправляем ID пакета
            LoRa.endPacket();
            xSemaphoreGive(loraMutex);

            uint32_t duration = millis() - startTime;
            Serial.printf("Hello packet %d sent, transmission time: %u ms\n", 
                         currentPacketId, duration);
            
            // Отмечаем, что пакет отправлен, но пока не подтвержден
            updateStats(false);
            blinkLED(3, 50, 255, 0, 0); // Красный
        }
        vTaskDelay(pdMS_TO_TICKS(random(15000, 30000)));
    }
}

void taskReceive(void *parameter) {
    for (;;) {
        if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(5000))) {
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
                    xSemaphoreGive(loraMutex);
                    uint32_t duration = millis() - startTime;
                    Serial.printf("ACK packet sent, transmission time: %u ms\n", duration);
                    blinkLED(2, 1000, 0, 255, 0); // Зелёный
                } else if (incoming.startsWith("ACK:")) {
                    // Получили подтверждение
                    int ackId = incoming.substring(4).toInt();
                    Serial.printf("ACK received for packet %d!\n", ackId);
                    
                    // Обновляем статистику только для этого пакета
                    updatePacketStatus(ackId, true);
                    xSemaphoreGive(loraMutex);
                    blinkLED(2, 1000, 0, 0, 255); // Синий
                } else {
                    xSemaphoreGive(loraMutex);
                }
            } else {
                xSemaphoreGive(loraMutex);
            }
        } else {
          logger.println("Failed to acquire mutex for receive!");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Заменяем существующую функцию taskMonitorStack
void taskMonitorStack(void *parameter) {
    // Создаем экземпляр SystemMonitor, если он еще не создан
    if (systemMonitor == nullptr) {
        systemMonitor = new SystemMonitor();
    }
    
    for (;;) {
        // Обновляем данные мониторинга
        systemMonitor->update();
        
        // Логируем базовую информацию о стеке
        UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("Free stack: %d bytes\n", freeStack);
        
        // Периодически логируем полную статистику (раз в минуту)
        static uint8_t fullLogCounter = 0;
        if (++fullLogCounter >= 6) {  // 6 * 10 секунд = 1 минута
            systemMonitor->logTasksStatistics();
            systemMonitor->logMemoryStatistics();
            fullLogCounter = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000));  // Проверка каждые 10 секунд
    }
}

// Задача для обработки веб-интерфейса
void taskWebInterface(void *parameter) {
    for (;;) {
        sett.tick();
        
        // Обновление данных для графика каждые 500 мс
        static uint32_t plotTimer = 0;
        if (millis() - plotTimer >= 500) {
            plotTimer = millis();
            plotManager.updateData();
        }
        
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
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Небольшая задержка для экономии ресурсов
    }
}

void taskDisplayUpdate(void *parameter) {
    for (;;) {
        if (displayManager && displayManager->isEnabled()) {
            displayManager->tick();
            displayManager->updateCurrentPage();
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Обновление каждые 50 мс
    }
}