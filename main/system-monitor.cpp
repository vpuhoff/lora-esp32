#include "system-monitor.h"
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <Arduino.h>

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
    
    // Обновляем простую метрику CPU на основе статистики выполнения задач
    updateCpuUsage();
    
    _lastUpdateTime = millis();
}

void SystemMonitor::updateCpuUsage() {
    static uint32_t idleCycles = 0;
    static uint32_t lastMeasurementTime = 0;
    static uint32_t maxIdleCycles = 0;
    static uint32_t totalSamples = 0;
    
    // We'll create a moving average window for our max idle cycles
    // to adapt to changing system conditions
    const uint32_t WINDOW_SIZE = 10;  // Number of samples to average
    
    // Only update once per second to avoid frequent calculations
    if (millis() - lastMeasurementTime > 1000) {
        // Calculate the load based on idle cycles
        if (totalSamples > 0) {
            // Use exponential moving average to smooth the max value
            if (idleCycles > maxIdleCycles) {
                maxIdleCycles = (idleCycles * 3 + maxIdleCycles * 7) / 10;
            } else if (totalSamples > WINDOW_SIZE) {
                // Gradually reduce maxIdleCycles if we're consistently seeing lower values
                // This helps the system adapt if it becomes less busy
                maxIdleCycles = (maxIdleCycles * 99) / 100;
            }
            
            // Calculate CPU usage as percentage of max idle time observed
            if (maxIdleCycles > 0) {
                uint32_t idlePercentage = (idleCycles * 100) / maxIdleCycles;
                _cpuUsageTotal = constrain(100 - idlePercentage, 0, 100);
            }
        } else {
            // First measurement, establish baseline
            maxIdleCycles = idleCycles;
            _cpuUsageTotal = 0;
        }
        
        totalSamples++;
        idleCycles = 0;
        lastMeasurementTime = millis();
    } else {
        // Increment idle cycle counter
        idleCycles++;
    }
}


String SystemMonitor::getFormattedTasksInfo() {
    String result = "Tasks: " + String(_taskCount) + "\n";
    result += "CPU: " + String(_cpuUsageTotal) + "%\n";
    result += "Free Heap: " + String(_freeHeap / 1024) + " kB\n";
    result += "Min Free: " + String(_minFreeHeap / 1024) + " kB\n";
    
    // Дополнительно можем собрать информацию о стеке текущей задачи
    UBaseType_t currentTaskStack = uxTaskGetStackHighWaterMark(NULL);
    result += "Current Task Stack: " + String(currentTaskStack) + " B\n";
    
    return result;
}

SystemMonitor::TaskInfo* SystemMonitor::getTasksInfo(uint16_t& count) {
    // Возвращаем только один элемент
    count = 1; // Важно! Устанавливаем count = 1, а не _taskCount
    
    TaskInfo* taskInfoArray = (TaskInfo*)pvPortMalloc(sizeof(TaskInfo));
    
    if (taskInfoArray != nullptr) {
        // Заполняем информацией о системе в целом
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
    // В этой упрощенной реализации мы не можем получить информацию
    // о конкретной задаче по имени без uxTaskGetSystemState
    return false;
}

void SystemMonitor::logTasksStatistics() {
    logger.println("---- System Statistics ----");
    logger.println("Tasks: " + String(_taskCount));
    logger.println("CPU Usage: " + String(_cpuUsageTotal) + "%");
    logger.println("Free Heap: " + String(_freeHeap / 1024) + " kB");
    logger.println("Min Free Heap: " + String(_minFreeHeap / 1024) + " kB");
    
    // Попробуем использовать vTaskList, если она доступна
#if configUSE_STATS_FORMATTING_FUNCTIONS == 1
    char buffer[500];
    vTaskList(buffer);
    logger.println("Task List:");
    logger.println(buffer);
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