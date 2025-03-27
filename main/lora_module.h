#ifndef LORA_MODULE_H
#define LORA_MODULE_H

#include "config.h"
#include <LoRa.h>
#include "logging.h"

// Инициализация LoRa модуля
bool setupLoRa();
void restartLoRaModule();

#endif // LORA_MODULE_H 