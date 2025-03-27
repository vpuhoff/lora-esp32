#ifndef TASKS_H
#define TASKS_H

#include "config.h"
#include <LoRa.h>  // Подключаем библиотеку LoRa
#include "logging.h"

// Создание всех задач
void createTasks();

// Задача отправки "Hello" сообщений
void taskSendHello(void *parameter);

// Задача приема сообщений
void taskReceive(void *parameter);

void taskLoraRestart(void * parameter);

// Задача мониторинга стека
void taskMonitorStack(void *parameter);

// Задача для обработки веб-интерфейса
void taskWebInterface(void *parameter);

// Задача обновления дисплея
void taskDisplayUpdate(void *parameter);

#endif // TASKS_H