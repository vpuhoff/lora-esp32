#include "plot-manager.h"
#include "statistics.h"

// Глобальный экземпляр менеджера графика
PlotManager plotManager;

// Конструктор
PlotManager::PlotManager() : _isUpdated(false) {
    // Инициализация массива данных
    for (int i = 0; i < MAX_PLOT_POINTS; i++) {
        _plotData[i] = 50;  // Начальное значение
    }
    _plotIndex = 0;
}

// Обновление данных графика на основе статистики
void PlotManager::updateData() {
    // Добавляем новую точку на основе сглаженной статистики успешности доставки
    _plotData[_plotIndex] = constrain(successRateSmoothed, 0, 100);
    
    // Обновляем индекс
    _plotIndex = (_plotIndex + 1) % MAX_PLOT_POINTS;
    
    // Устанавливаем флаг обновления
    _isUpdated = true;
}

// Формирование JSON для отображения графика
String PlotManager::getPlotJson() {
    String plotJson = "[";
    for (int i = 0; i < MAX_PLOT_POINTS; i++) {
        int idx = (_plotIndex + i) % MAX_PLOT_POINTS;
        plotJson += String(_plotData[idx]);
        if (i < MAX_PLOT_POINTS - 1) plotJson += ",";
    }
    plotJson += "]";
    
    return plotJson;
}

// Проверка обновления и сброс флага
bool PlotManager::checkAndResetUpdate() {
    bool result = _isUpdated;
    _isUpdated = false;
    return result;
}