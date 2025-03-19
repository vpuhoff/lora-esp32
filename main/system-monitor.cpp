#include "system-monitor.h"
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <Arduino.h>
#include <string.h>

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
    // Получаем массив информации о задачах
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t* taskStatusArray = (TaskStatus_t*) pvPortMalloc(taskCount * sizeof(TaskStatus_t));
    if (taskStatusArray == nullptr) {
        _cpuUsageTotal = 0;
        return;
    }
    uint32_t totalRunTime = 0;
    // Функция заполняет массив и возвращает суммарное время работы всех задач
    taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);
    
    uint32_t idleRunTime = 0;
    // На ESP32 обычно два idle-задачи – суммируем время обеих
    for (UBaseType_t i = 0; i < taskCount; i++) {
        if (taskStatusArray[i].pcTaskName != nullptr &&
          strstr(taskStatusArray[i].pcTaskName, "IDLE") != nullptr) {
          idleRunTime += taskStatusArray[i].ulRunTimeCounter;
      }
    }
    if (totalRunTime > 0) {
        uint32_t idlePercentage = (idleRunTime * 100UL) / totalRunTime;
        _cpuUsageTotal = constrain(100 - idlePercentage, 0, 100);
    } else {
        _cpuUsageTotal = 0;
    }
    vPortFree(taskStatusArray);
#else
    // Если статистика времени не включена, ставим 0%
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
    // Возвращаем только один элемент
    count = 1;
    TaskInfo* taskInfoArray = (TaskInfo*)pvPortMalloc(sizeof(TaskInfo));
    if (taskInfoArray != nullptr) {
        strncpy(taskInfoArray[0].name, "System", sizeof(taskInfoArray[0].name) - 1);
        taskInfoArray[0].name[sizeof(taskInfoArray[0].name) - 1] = '\0';
        taskInfoArray[0].handle = NULL;
        taskInfoArray[0].priority = 0;
        taskInfoArray[0].stackHighWater = 0;
        taskInfoArray[0].state = eRunning;
        taskInfoArray[0].cpuUsage = _cpuUsageTotal;
    }
    return taskInfoArray;
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
        case eRunning:   return "RUN";
        case eReady:     return "READY";
        case eBlocked:   return "BLOCK";
        case eSuspended: return "SUSP";
        case eDeleted:   return "DEL";
        default:         return "UNK";
    }
}