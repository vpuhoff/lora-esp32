#include "lora_module.h"
#include "led.h"

bool setupLoRa() {
    LoRa.setPins(SS, RST, DIO0);
    logger.println("Initializing LoRa...");
    int attempts = 0;
    while (!LoRa.begin(LORA_FREQUENCY) && attempts < LORA_MAX_ATTEMPTS) {
        logger.println("LoRa init failed, retrying...");
        blinkLED(2, 200);
        vTaskDelay(pdMS_TO_TICKS(10000));
        attempts++;
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