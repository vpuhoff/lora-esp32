#pragma once
#include <Arduino.h>
#include "esp32-config.h"
#include "logger.h"

class PlotManager {
public:
    PlotManager();
    
    // Обновление данных графика
    void updateData();
    
    // Формирование JSON для отображения графика
    String getPlotJson();
    
    // Проверка обновления и сброс флага
    bool checkAndResetUpdate();

private:
    uint32_t _plotData[MAX_PLOT_POINTS]; // Данные для графика
    int _plotIndex;                      // Текущий индекс в массиве данных
    bool _isUpdated;                     // Флаг обновления графика
};

// Глобальный экземпляр менеджера графика
extern PlotManager plotManager;