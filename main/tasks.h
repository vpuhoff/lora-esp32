#ifndef TASKS_H
#define TASKS_H

#include "config.h"
#include <LoRa.h>  // Подключаем библиотеку LoRa

// Создание всех задач
void createTasks();

// Задача отправки "Hello" сообщений
void taskSendHello(void *parameter);

// Задача приема сообщений
void taskReceive(void *parameter);

// Задача мониторинга стека
void taskMonitorStack(void *parameter);

#endif // TASKS_H