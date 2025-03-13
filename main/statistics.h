#ifndef STATISTICS_H
#define STATISTICS_H

#include "config.h"

// Функция обновления статистики
void updateStats(bool success);

// Обновляем статус для конкретного пакета
void updatePacketStatus(int id, bool success);

// Глобальные переменные для отслеживания статистики
extern int totalSent;
extern int totalReceived;
extern float successRateSmoothed;

// Глобальные переменные для отслеживания пакетов
extern int packetId;
extern bool packetStatus[100]; // Для хранения статуса пакетов

#endif // STATISTICS_H