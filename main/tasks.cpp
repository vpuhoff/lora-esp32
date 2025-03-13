#include "tasks.h"
#include "led.h"
#include "statistics.h"

void createTasks() {
    xTaskCreatePinnedToCore(taskSendHello, "SendHello", 3072, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskReceive, "Receive", 3072, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskMonitorStack, "StackMonitor", 2048, NULL, 1, NULL, 1);
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
                    
                    Serial.println("Hello received! Sending ACK...");
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
          Serial.println("Failed to acquire mutex for receive!");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void taskMonitorStack(void *parameter) {
    for (;;) {
        UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("Free stack: %d bytes\n", freeStack);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}