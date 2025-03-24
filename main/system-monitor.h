#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "logging.h"


class SystemMonitor {
public:
    SystemMonitor();
    // Структура для хранения информации о задаче
    struct TaskInfo {
        char name[32];         // Имя задачи
        TaskHandle_t handle;     // Указатель на задачу
        UBaseType_t priority;   // Приоритет
        size_t stackHighWater; // Минимальный свободный стек
        eTaskState state;       // Состояние задачи
        float cpuUsage;         // Использование CPU в процентах
    };

    // Основные функции мониторинга
    void update();                                     // Сбор данных о системе
    String getFormattedTasksInfo();                   // Получение форматированной статистики задач
    TaskInfo* getTasksInfo(uint16_t& count);          // Получение массива с информацией о задачах

    // Получение системной информации
    uint32_t getTotalCpuUsage();                      // Общее использование CPU
    uint32_t getFreeHeap();                            // Свободная память кучи
    uint32_t getMinFreeHeap();                        // Минимальная свободная память за время работы

    // Получение информации о конкретной задаче по имени
    bool getTaskInfoByName(const char* taskName, TaskInfo& taskInfo);

    // Вывод статистики в лог
    void logTasksStatistics();
    void logMemoryStatistics();

private:
    uint32_t _taskCount;       // Количество задач
    uint32_t _freeHeap;         // Свободная память
    uint32_t _minFreeHeap;     // Минимальная свободная память
    uint32_t _cpuUsageTotal;   // Общее использование CPU
    uint32_t _cpuUsageIdle;    // Использование CPU idle task
    uint32_t _lastUpdateTime;  // Время последнего обновления

    // Вспомогательные методы
    void updateCpuUsage();                            // Расчет загрузки CPU
    String taskStateToString(eTaskState state);       // Преобразование состояния задачи в строку

    // Функция для сравнения TaskInfo по cpuUsage (для сортировки)
    static bool compareTasks(const TaskInfo& a, const TaskInfo& b);
};

// Глобальный экземпляр класса мониторинга
extern SystemMonitor* systemMonitor;