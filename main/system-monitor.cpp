#include "system-monitor.h"
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <Arduino.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <algorithm>

// Функция для сравнения TaskInfo по cpuUsage (для сортировки)
bool SystemMonitor::compareTasks(const SystemMonitor::TaskInfo& a, const SystemMonitor::TaskInfo& b) {
    return a.cpuUsage > b.cpuUsage;
}

extern SystemMonitor* systemMonitor;

SystemMonitor::SystemMonitor() {
    _taskCount = 0;
    _lastUpdateTime = 0;
    _cpuUsageIdle = 0;
    _cpuUsageTotal = 0;
}

void SystemMonitor::update() {
    // Обновляем общую информацию о системе
    _freeHeap = esp_get_free_heap_size();
    _minFreeHeap = esp_get_minimum_free_heap_size();

    // Получаем количество задач
    _taskCount = uxTaskGetNumberOfTasks();

    // Обновляем метрику CPU, используя статистику FreeRTOS runtime stats
    updateCpuUsage();

    _lastUpdateTime = millis();
}

void SystemMonitor::updateCpuUsage() {
#if configGENERATE_RUN_TIME_STATS == 1
    // Получаем количество задач
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    if (taskCount == 0) {
        _cpuUsageTotal = 0;
        return;
    }

    // Выделяем массив под статусы
    TaskStatus_t* taskStatusArray = (TaskStatus_t*) pvPortMalloc(taskCount * sizeof(TaskStatus_t));
    if (taskStatusArray == nullptr) {
        _cpuUsageTotal = 0;
        return;
    }

    uint32_t totalRunTime = 0;
    // Получаем состояние задач и суммарное время
    taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);

    // Суммируем время для всех idle-задач (на ESP32 обычно их две: IDLE0 и IDLE1)
    uint32_t idleRunTime = 0;
    for (UBaseType_t i = 0; i < taskCount; i++) {
        if (taskStatusArray[i].pcTaskName != nullptr &&
            strstr(taskStatusArray[i].pcTaskName, "IDLE") != nullptr) {
            idleRunTime += taskStatusArray[i].ulRunTimeCounter;
        }
    }

    vPortFree(taskStatusArray);

    // Если totalRunTime == 0, статистика не успела накопиться
    if (totalRunTime > 0) {
        // Вычисляем процент времени в idle и вычитаем из 100
        uint32_t idlePercentage = (idleRunTime * 100UL) / totalRunTime;
        if (idlePercentage >= 100) {
            // На случай, если из-за округлений idlePercentage может выйти >100
            _cpuUsageTotal = 0;
        } else {
            _cpuUsageTotal = 100 - idlePercentage;
        }
    } else {
        _cpuUsageTotal = 0;
    }
#else
    // Если сбор статистики не включён в конфигурации FreeRTOS
    _cpuUsageTotal = 0;
#endif
}


String SystemMonitor::getFormattedTasksInfo() {
    String result = "Tasks: " + String(_taskCount) + "\n";
    result += "CPU: " + String(_cpuUsageTotal) + "%\n";
    result += "Free Heap: " + String(_freeHeap / 1024) + " kB\n";
    result += "Min Free: " + String(_minFreeHeap / 1024) + " kB\n";
    
    // // Дополнительно: информация о стеке текущей задачи
    // UBaseType_t currentTaskStack = uxTaskGetStackHighWaterMark(NULL);
    // result += "Current Task Stack: " + String(currentTaskStack) + " B\n";

    return result;
}

SystemMonitor::TaskInfo* SystemMonitor::getTasksInfo(uint16_t& count) {
    // Статический буфер для результатов (чтобы не возвращать указатель на локальный массив)
    static SystemMonitor::TaskInfo s_taskInfo[configMAX_PRIORITIES];

    // Сначала узнаём, сколько задач
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
    if (uxArraySize == 0) {
        count = 0;
        return s_taskInfo;
    }

    // Выделяем массив под TaskStatus_t
    TaskStatus_t* pxTaskStatusArray = (TaskStatus_t*) pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if (pxTaskStatusArray == nullptr) {
        count = 0;
        return s_taskInfo;
    }

    // Запросим у системы массив статусов задач и суммарное время
    uint32_t totalRunTime = 0;
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &totalRunTime);

    // Заполняем наш массив s_taskInfo
    uint16_t taskCount = 0;
    for (UBaseType_t i = 0; i < uxArraySize; i++) {
        if (taskCount < configMAX_PRIORITIES) {
            const TaskStatus_t& src = pxTaskStatusArray[i];
            SystemMonitor::TaskInfo& dst = s_taskInfo[taskCount];

            // Имя задачи
            strncpy(dst.name, pcTaskGetName(src.xHandle), sizeof(dst.name) - 1);
            dst.name[sizeof(dst.name) - 1] = '\0';

            dst.handle = src.xHandle;
            dst.priority = uxTaskPriorityGet(src.xHandle);
            dst.stackHighWater = uxTaskGetStackHighWaterMark(src.xHandle);
            dst.state = src.eCurrentState;

            // Вычисляем процент CPU, если totalRunTime > 0
            if (totalRunTime > 0) {
                dst.cpuUsage = (static_cast<float>(src.ulRunTimeCounter) * 100.0f) 
                               / static_cast<float>(totalRunTime);
            } else {
                dst.cpuUsage = 0.0f;
            }

            taskCount++;
        }
    }

    // Освобождаем временный массив
    vPortFree(pxTaskStatusArray);

    // Сортируем задачи по убыванию cpuUsage (пузырьковая сортировка для простоты)
    for (int i = 0; i < taskCount - 1; i++) {
        for (int j = 0; j < taskCount - i - 1; j++) {
            if (s_taskInfo[j].cpuUsage < s_taskInfo[j + 1].cpuUsage) {
                SystemMonitor::TaskInfo temp = s_taskInfo[j];
                s_taskInfo[j] = s_taskInfo[j + 1];
                s_taskInfo[j + 1] = temp;
            }
        }
    }

    // Возвращаем не более 20 задач
    count = std::min<uint16_t>(20, taskCount);
    return s_taskInfo;
}


uint32_t SystemMonitor::getTotalCpuUsage() {
    return _cpuUsageTotal;
}

uint32_t SystemMonitor::getFreeHeap() {
    return _freeHeap;
}

uint32_t SystemMonitor::getMinFreeHeap() {
    return _minFreeHeap;
}

bool SystemMonitor::getTaskInfoByName(const char* taskName, TaskInfo& taskInfo) {
    // В данной реализации не реализовано получение информации по имени
    return false;
}

void SystemMonitor::logTasksStatistics() {
    logger.println("---- System Statistics ----");
    logger.println("Tasks: " + String(_taskCount));
    logger.println("CPU Usage: " + String(_cpuUsageTotal) + "%");
    logger.println("Free Heap: " + String(_freeHeap / 1024) + " kB");
    logger.println("Min Free Heap: " + String(_minFreeHeap / 1024) + " kB");

#if configUSE_STATS_FORMATTING_FUNCTIONS == 1
    // char buffer[2500];
    // vTaskList(buffer);
    // Serial.println("Task List:");
    // logger.println(buffer);
#endif

    logger.println("-------------------------");
}

void SystemMonitor::logMemoryStatistics() {
    logger.println("---- Memory Statistics ----");
    logger.println("Free Heap: " + String(_freeHeap / 1024) + " kB");
    logger.println("Min Free Heap: " + String(_minFreeHeap / 1024) + " kB");

#if defined(CONFIG_SPIRAM_SUPPORT)
    logger.println("PSRAM Size: " + String(esp_spiram_get_size() / 1024) + " kB");
    logger.println("Free PSRAM: " + String(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024) + " kB");
#endif

    logger.println("--------------------------");
}

String SystemMonitor::taskStateToString(eTaskState state) {
    switch (state) {
        case eRunning:    return "RUN";
        case eReady:      return "READY";
        case eBlocked:    return "BLOCK";
        case eSuspended:  return "SUSP";
        case eDeleted:    return "DEL";
        default:          return "UNK";
    }
}