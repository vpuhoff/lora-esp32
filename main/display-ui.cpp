#include "display-ui.h"
#include "statistics.h"
#include "plot-manager.h"
#include "system-monitor.h"

namespace DisplayUI {

// Функции для отрисовки элементов интерфейса
void drawProgressBar(Adafruit_ST7735* display, int x, int y, int width, int height, int progress, uint16_t barColor, uint16_t bgColor) {
    // Ограничиваем значение прогресса до 0-100%
    progress = constrain(progress, 0, 100);
    
    // Рисуем фон
    display->fillRect(x, y, width, height, bgColor);
    
    // Рисуем прогресс
    int progressWidth = (width * progress) / 100;
    if (progressWidth > 0) {
        display->fillRect(x, y, progressWidth, height, barColor);
    }
    
    // Рисуем рамку
    display->drawRect(x, y, width, height, COLOR_TEXT);
}

void drawTextBox(Adafruit_ST7735* display, int x, int y, int width, int height, String text, uint16_t borderColor, uint16_t textColor) {
    // Рисуем рамку
    display->drawRect(x, y, width, height, borderColor);
    
    // Устанавливаем параметры текста
    display->setTextColor(textColor);
    display->setTextSize(1);
    
    // Ограничиваем область текста внутренними границами
    x += 2;
    y += 2;
    width -= 4;
    height -= 4;
    
    // Разбиваем текст на строки и отображаем
    int16_t cursorX = x;
    int16_t cursorY = y;
    int16_t lineHeight = 8;  // Высота строки для размера текста 1
    
    for (unsigned int i = 0; i < text.length(); i++) {
        // Проверяем, нужно ли переносить строку
        if (text[i] == '\n' || (cursorX + 6 > x + width)) {  // 6 - примерная ширина символа
            cursorX = x;
            cursorY += lineHeight;
            
            // Если достигли нижней границы, останавливаемся
            if (cursorY + lineHeight > y + height) {
                break;
            }
            
            // Пропускаем символ новой строки
            if (text[i] == '\n') {
                continue;
            }
        }
        
        // Отображаем символ
        display->setCursor(cursorX, cursorY);
        display->print(text[i]);
        cursorX += 6;  // Примерная ширина символа
    }
}

void drawPageIndicator(Adafruit_ST7735* display, int totalPages, int currentPage, int yPosition) {
    int dotSize = 4;
    int spacing = 8;
    int totalWidth = totalPages * dotSize + (totalPages - 1) * spacing;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    for (int i = 0; i < totalPages; i++) {
        int x = startX + i * (dotSize + spacing);
        if (i == currentPage) {
            display->fillCircle(x + dotSize/2, yPosition + dotSize/2, dotSize/2, COLOR_SUCCESS);
        } else {
            display->drawCircle(x + dotSize/2, yPosition + dotSize/2, dotSize/2, COLOR_TEXT);
        }
    }
}

void drawStatusBar(Adafruit_ST7735* display, bool wifiConnected, bool loraActive, int batteryLevel) {
    // Рисуем фон статус-бара
    display->fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_HEADER);
    
    // Отображаем индикатор WiFi
    if (wifiConnected) {
        drawWiFiSignal(display, 4, 2, WiFi.RSSI(), ST7735_WHITE);
    } else {
        // Перечеркнутый WiFi символ (не подключен)
        display->drawLine(4, 2, 10, 8, ST7735_WHITE);
    }
    
    // Отображаем индикатор LoRa
    if (loraActive) {
        display->fillRect(15, 3, 3, 4, ST7735_WHITE);
        display->fillRect(19, 2, 3, 5, ST7735_WHITE);
        display->fillRect(23, 1, 3, 6, ST7735_WHITE);
    } else {
        display->drawRect(15, 3, 3, 4, 0x630C); // тусклый красный
        display->drawRect(19, 2, 3, 5, 0x630C);
        display->drawRect(23, 1, 3, 6, 0x630C);
    }
    
    // Отображаем уровень батареи, если указан
    if (batteryLevel >= 0) {
        drawBattery(display, SCREEN_WIDTH - 15, 2, batteryLevel);
    }
    
    // Отображаем время работы
    display->setTextColor(ST7735_WHITE);
    display->setTextSize(1);
    display->setCursor((SCREEN_WIDTH / 2) - 15, 1);
    display->print(formatUptime(millis()));
}

// Функции для отрисовки страниц
void drawLogoPage(Adafruit_ST7735* display, const char* version) {
    // Заголовок
    drawHeader(display, "ESP32 LoRa");
    
    // Логотип (упрощенный вариант - просто текст)
    display->setTextSize(2);
    display->setTextColor(COLOR_SUCCESS);
    drawCenteredText(display, 40, "LoRa", COLOR_SUCCESS, 2);
    drawCenteredText(display, 60, "Control", COLOR_SUCCESS, 2);
    drawCenteredText(display, 80, "Panel", COLOR_SUCCESS, 2);
    
    // Версия
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT);
    drawCenteredText(display, 105, String("v") + version);
}

void drawLoRaStatusPage(Adafruit_ST7735* display) {
    // Заголовок
    drawHeader(display, "LoRa Status");
    
    if (loraManager == nullptr) {
        drawCenteredText(display, 60, "LoRa Manager", COLOR_TEXT);
        drawCenteredText(display, 70, "not available", COLOR_ERROR);
        return;
    }
    
    // Настройки LoRa
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    display->setCursor(5, 15);
    display->print("SF: ");
    display->print(loraManager->getSpreadingFactor());
    
    display->setCursor(50, 15);
    display->print("BW: ");
    display->print(loraManager->getBandwidth());
    display->print(" kHz");
    
    display->setCursor(5, 25);
    display->print("CR: 4/");
    display->print(loraManager->getCodingRate());
    
    display->setCursor(50, 25);
    display->print("TX: ");
    display->print(loraManager->getTxPower());
    display->print(" dBm");
    
    // Статистика пакетов
    display->setCursor(5, 40);
    display->print("Packets: ");
    display->print(loraManager->getPacketsTotal());
    
    display->setCursor(5, 50);
    display->print("Delivered: ");
    display->print(loraManager->getPacketsSuccess());
    
    // Процент успешной доставки
    display->setCursor(5, 60);
    display->print("Success rate: ");
    display->print(loraManager->getSuccessRate());
    display->print("%");
    
    // Отображаем RSSI
    display->setCursor(5, 70);
    display->print("RSSI: ");
    display->print(loraManager->getLastRssi(), 1);
    display->print(" dBm");
    
    // Рисуем прогресс-бар для успешности доставки
    drawProgressBar(display, 5, 85, SCREEN_WIDTH - 10, 10, loraManager->getSuccessRate());
    
    // График успешности доставки (используем данные из plotManager)
    display->drawRect(5, 100, SCREEN_WIDTH - 10, 20, COLOR_TEXT);
    
    // Преобразуем данные для миниграфика
    uint32_t graphData[20];  // Используем 20 последних точек
    for (int i = 0; i < 20; i++) {
        graphData[i] = 50;  // Заполняем значением по умолчанию
    }
    
    // Отрисовываем миниграфик
    drawMiniGraph(display, 5, 100, SCREEN_WIDTH - 10, 20, graphData, 20);
}

void drawWiFiStatusPage(Adafruit_ST7735* display) {
    // Заголовок
    drawHeader(display, "WiFi Status");
    
    if (wifiManager == nullptr) {
        drawCenteredText(display, 60, "WiFi Manager", COLOR_TEXT);
        drawCenteredText(display, 70, "not available", COLOR_ERROR);
        return;
    }
    
    // Получаем текст статуса WiFi
    String wifiStatus = wifiManager->getStatusText();
    
    // Разбиваем строку статуса на отдельные строки
    int startPos = 0;
    int lineNum = 0;
    while (startPos < wifiStatus.length() && lineNum < 5) {
        int endPos = wifiStatus.indexOf('\n', startPos);
        if (endPos == -1) {
            endPos = wifiStatus.length();
        }
        
        String line = wifiStatus.substring(startPos, endPos);
        
        display->setTextColor(COLOR_TEXT);
        display->setTextSize(1);
        display->setCursor(5, 15 + lineNum * 10);
        display->print(line);
        
        startPos = endPos + 1;
        lineNum++;
    }
    
    // Отображаем силу сигнала для режима STA
    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
        if (WiFi.status() == WL_CONNECTED) {
            int rssi = WiFi.RSSI();
            int signalStrength = map(constrain(rssi, -90, -30), -90, -30, 0, 100);
            
            display->setCursor(5, 65);
            display->print("Signal strength: ");
            display->print(signalStrength);
            display->print("%");
            
            // Рисуем прогресс-бар силы сигнала
            uint16_t barColor = COLOR_ERROR;
            if (signalStrength > 70) barColor = COLOR_SUCCESS;
            else if (signalStrength > 30) barColor = COLOR_WARNING;
            
            drawProgressBar(display, 5, 75, SCREEN_WIDTH - 10, 10, signalStrength, barColor);
            
            // Отображаем RSSI
            display->setCursor(5, 90);
            display->print("RSSI: ");
            display->print(rssi);
            display->print(" dBm");
            
            // Большой индикатор силы сигнала
            int barCount = map(signalStrength, 0, 100, 0, 4);
            for (int i = 0; i < 4; i++) {
                int barHeight = (i + 1) * 5;
                int barX = (SCREEN_WIDTH / 2) - 20 + i * 10;
                int barY = 110 - barHeight;
                int barWidth = 6;
                
                if (i < barCount) {
                    // Активная полоса
                    display->fillRect(barX, barY, barWidth, barHeight, barColor);
                } else {
                    // Неактивная полоса
                    display->drawRect(barX, barY, barWidth, barHeight, COLOR_TEXT);
                }
            }
        }
    }
}

void drawSystemInfoPage(Adafruit_ST7735* display) {
    // Заголовок
    drawHeader(display, "System Info");
    
    // Время работы
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    display->setCursor(5, 15);
    display->print("Uptime: ");
    display->print(formatUptime(millis()));
    
    // Свободная память
    display->setCursor(5, 25);
    display->print("Free RAM: ");
    display->print(ESP.getFreeHeap() / 1024);
    display->print(" kB");
    
    // Минимальная свободная память
    display->setCursor(5, 35);
    display->print("Min Free: ");
    display->print(ESP.getMinFreeHeap() / 1024);
    display->print(" kB");
    
    // Загрузка CPU (если доступен системный монитор)
    if (systemMonitor != nullptr) {
        systemMonitor->update();
        
        // Общая загрузка CPU
        display->setCursor(5, 50);
        display->print("CPU Usage: ");
        display->print(systemMonitor->getTotalCpuUsage());
        display->print("%");
        
        // Прогресс-бар загрузки CPU
        uint16_t cpuBarColor = COLOR_SUCCESS;
        uint32_t cpuUsage = systemMonitor->getTotalCpuUsage();
        
        if (cpuUsage > 80) cpuBarColor = COLOR_ERROR;
        else if (cpuUsage > 50) cpuBarColor = COLOR_WARNING;
        
        drawProgressBar(display, 5, 60, SCREEN_WIDTH - 10, 10, cpuUsage, cpuBarColor);
        
        // Отображаем информацию о задачах
        display->setCursor(5, 75);
        display->print("Tasks:");
        
        uint16_t taskCount = 0;
        SystemMonitor::TaskInfo* tasks = systemMonitor->getTasksInfo(taskCount);
        
        if (tasks != nullptr && taskCount > 0) {
            int y = 85;
            
            // Отображаем только 3 наиболее требовательные задачи
            // Сортируем задачи по загрузке CPU
            for (uint16_t i = 0; i < taskCount - 1; i++) {
                for (uint16_t j = i + 1; j < taskCount; j++) {
                    if (tasks[j].cpuUsage > tasks[i].cpuUsage) {
                        // Свап
                        SystemMonitor::TaskInfo temp = tasks[i];
                        tasks[i] = tasks[j];
                        tasks[j] = temp;
                    }
                }
            }
            
            // Отображаем топ-3 задачи
            for (uint16_t i = 0; i < taskCount && i < 3; i++) {
                // Пропускаем системные задачи
                if (strcmp(tasks[i].name, "IDLE") == 0 ||
                    strncmp(tasks[i].name, "tiT", 3) == 0) {
                    continue;
                }
                
                display->setCursor(5, y);
                display->print(tasks[i].name);
                
                display->setCursor(80, y);
                display->print(tasks[i].cpuUsage);
                display->print("%");
                
                y += 10;
                if (y > 115) break;
            }
            
            // Освобождаем память
            vPortFree(tasks);
        }
    } else {
        // Если системный монитор недоступен
        display->setCursor(5, 50);
        display->print("CPU monitoring not available");
    }
}

void drawLogsPage(Adafruit_ST7735* display, const String* logLines, int lineCount) {
    // Заголовок
    drawHeader(display, "System Logs");
    
    // Проверяем, есть ли логи для отображения
    if (lineCount == 0) {
        drawCenteredText(display, 60, "No logs available");
        return;
    }
    
    // Отображаем строки лога
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    for (int i = 0; i < lineCount && i < 10; i++) {  // Ограничиваем 10 строками
        String line = logLines[i];
        
        // Определяем цвет в зависимости от типа лога
        uint16_t color = COLOR_TEXT;
        if (line.startsWith("err:") || line.startsWith("error:")) {
            color = COLOR_ERROR;
        } else if (line.startsWith("warn:") || line.startsWith("warning:")) {
            color = COLOR_WARNING;
        } else if (line.startsWith("info:")) {
            color = COLOR_SUCCESS;
        }
        
        // Усекаем строку, если она слишком длинная
        if (line.length() > 21) {  // Примерно 21 символ помещается в строку
            line = line.substring(0, 19) + "..";
        }
        
        display->setTextColor(color);
        display->setCursor(5, 15 + i * 10);
        display->print(line);
    }
}

void drawCpuMonitorPage(Adafruit_ST7735* display) {
    // Заголовок
    drawHeader(display, "CPU Monitor");
    
    if (systemMonitor == nullptr) {
        drawCenteredText(display, 64, "CPU Monitor", COLOR_TEXT);
        drawCenteredText(display, 84, "Not available", COLOR_ERROR);
        return;
    }
    
    // Обновляем данные мониторинга
    systemMonitor->update();
    
    // Заголовок страницы
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT);
    display->setCursor(5, 15);
    display->print("CPU: ");
    display->print(systemMonitor->getTotalCpuUsage());
    display->print("% Free: ");
    display->print(systemMonitor->getFreeHeap() / 1024);
    display->print("kB");
    
    // Рисуем разделительную линию
    display->drawLine(0, 25, 128, 25, COLOR_HEADER);
    
    // Получаем информацию о задачах
    uint16_t taskCount = 0;
    SystemMonitor::TaskInfo* tasks = systemMonitor->getTasksInfo(taskCount);
    
    if (tasks != nullptr && taskCount > 0) {
        // Отображаем до 6 основных задач
        uint8_t displayLimit = min(6, (int)taskCount);
        
        display->setCursor(5, 30);
        display->println("Task      CPU  Stack");
        
        int y = 40;
        for (uint8_t i = 0; i < displayLimit; i++) {
            // Пропускаем системные задачи, если их много
            if (taskCount > 6 && 
                (strcmp(tasks[i].name, "IDLE") == 0 ||
                 strncmp(tasks[i].name, "tiT", 3) == 0)) {
                continue;
            }
            
            display->setCursor(5, y);
            
            // Имя задачи (сокращаем до 8 символов при необходимости)
            char shortName[9];
            strncpy(shortName, tasks[i].name, 8);
            shortName[8] = '\0';
            display->print(shortName);
            
            // Выравниваем позицию для CPU
            display->setCursor(70, y);
            display->print(tasks[i].cpuUsage);
            display->print("%");
            
            // Выравниваем позицию для стека
            display->setCursor(95, y);
            display->print(tasks[i].stackHighWater);
            
            // Определяем цвет для строки в зависимости от загрузки CPU
            uint16_t barColor;
            if (tasks[i].cpuUsage > 50) barColor = COLOR_ERROR;
            else if (tasks[i].cpuUsage > 20) barColor = COLOR_WARNING;
            else barColor = COLOR_SUCCESS;
            
            // Рисуем горизонтальный индикатор загрузки
            int barWidth = map(tasks[i].cpuUsage, 0, 100, 0, 60);
            display->drawRect(0, y, 3, 8, barColor);
            if (barWidth > 0) {
                display->fillRect(0, y, 3, 8, barColor);
            }
            
            y += 12;
            if (y > 110) break;  // Предотвращаем выход за границы экрана
        }
        
        // Освобождаем память
        vPortFree(tasks);
    } else {
        display->setCursor(5, 50);
        display->println("No task data available");
    }
}

// Функции для отображения сообщений
void drawInfoMessage(Adafruit_ST7735* display, String message) {
    // Заголовок
    drawHeader(display, "Information");
    
    // Рисуем рамку сообщения
    int boxWidth = SCREEN_WIDTH - 20;
    int boxHeight = 60;
    int boxX = 10;
    int boxY = 30;
    
    display->fillRect(boxX, boxY, boxWidth, boxHeight, COLOR_HEADER);
    display->drawRect(boxX, boxY, boxWidth, boxHeight, COLOR_TEXT);
    
    // Отображаем текст сообщения (с переносом строк)
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    // Разбиваем сообщение на строки
    int y = boxY + 10;
    int maxCharsPerLine = 20;  // Примерное количество символов в строке
    
    for (int i = 0; i < message.length(); i += maxCharsPerLine) {
        String line = message.substring(i, min((int)message.length(), i + maxCharsPerLine));
        
        // Если строка разрывает слово, ищем последний пробел
        if (i + maxCharsPerLine < message.length()) {
            int lastSpace = line.lastIndexOf(' ');
            if (lastSpace > 0) {
                line = line.substring(0, lastSpace);
                i = i - (maxCharsPerLine - lastSpace - 1);
            }
        }
        
        drawCenteredText(display, y, line);
        y += 10;
        
        if (y > boxY + boxHeight - 10) break;  // Предотвращаем выход за границы
    }
}

void drawErrorMessage(Adafruit_ST7735* display, String message) {
    // Заголовок
    drawHeader(display, "Error", COLOR_ERROR);
    
    // Рисуем рамку сообщения
    int boxWidth = SCREEN_WIDTH - 20;
    int boxHeight = 60;
    int boxX = 10;
    int boxY = 30;
    
    display->fillRect(boxX, boxY, boxWidth, boxHeight, 0x630C);  // Темно-красный фон
    display->drawRect(boxX, boxY, boxWidth, boxHeight, COLOR_ERROR);
    
    // Отображаем текст сообщения (с переносом строк)
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    // Разбиваем сообщение на строки
    int y = boxY + 10;
    int maxCharsPerLine = 20;  // Примерное количество символов в строке
    
    for (int i = 0; i < message.length(); i += maxCharsPerLine) {
        String line = message.substring(i, min((int)message.length(), i + maxCharsPerLine));
        
        // Если строка разрывает слово, ищем последний пробел
        if (i + maxCharsPerLine < message.length()) {
            int lastSpace = line.lastIndexOf(' ');
            if (lastSpace > 0) {
                line = line.substring(0, lastSpace);
                i = i - (maxCharsPerLine - lastSpace - 1);
            }
        }
        
        drawCenteredText(display, y, line);
        y += 10;
        
        if (y > boxY + boxHeight - 10) break;  // Предотвращаем выход за границы
    }
}

// Вспомогательные функции
void drawCenteredText(Adafruit_ST7735* display, int y, String text, uint16_t color, uint8_t size) {
    display->setTextColor(color);
    display->setTextSize(size);
    
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    int x = (SCREEN_WIDTH - w) / 2;
    
    display->setCursor(x, y);
    display->print(text);
}

void drawHeader(Adafruit_ST7735* display, String title, uint16_t color) {
    display->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, 10, color);
    
    display->setTextColor(ST7735_WHITE);
    display->setTextSize(1);
    
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int x = (SCREEN_WIDTH - w) / 2;
    
    display->setCursor(x, STATUS_BAR_HEIGHT + 2);
    display->print(title);
}

// Функция для отрисовки миниграфика
void drawMiniGraph(Adafruit_ST7735* display, int x, int y, int width, int height, const uint32_t* data, int dataPoints, uint16_t lineColor) {
    // Находим максимальное и минимальное значение для масштабирования
    uint32_t maxVal = 0;
    uint32_t minVal = UINT32_MAX;
    
    for (int i = 0; i < dataPoints; i++) {
        if (data[i] > maxVal) maxVal = data[i];
        if (data[i] < minVal) minVal = data[i];
    }
    
    // Если все значения одинаковые, добавляем немного диапазона
    if (maxVal == minVal) {
        maxVal += 10;
        minVal = (minVal > 10) ? minVal - 10 : 0;
    }
    
    // Рисуем линии графика
    for (int i = 1; i < dataPoints; i++) {
        int x1 = x + (width * (i - 1)) / dataPoints;
        int y1 = y + height - ((height * (data[i - 1] - minVal)) / (maxVal - minVal));
        
        int x2 = x + (width * i) / dataPoints;
        int y2 = y + height - ((height * (data[i] - minVal)) / (maxVal - minVal));
        
        // Ограничиваем координаты в пределах области рисования
        y1 = constrain(y1, y, y + height - 1);
        y2 = constrain(y2, y, y + height - 1);
        
        display->drawLine(x1, y1, x2, y2, lineColor);
    }
}

// Функция для форматирования IP-адреса
String formatIP(IPAddress ip) {
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

// Функция для форматирования времени работы
String formatUptime(uint32_t milliseconds) {
    uint32_t seconds = milliseconds / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    uint32_t days = hours / 24;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    char buffer[20];
    
    if (days > 0) {
        snprintf(buffer, sizeof(buffer), "%dd %02d:%02d:%02d", days, hours, minutes, seconds);
    } else {
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    }
    
    return String(buffer);
}

// Функция для отображения индикатора сигнала WiFi
void drawWiFiSignal(Adafruit_ST7735* display, int x, int y, int rssi, uint16_t color) {
    // Преобразуем RSSI в силу сигнала от 0 до 4
    int strength = 0;
    
    if (rssi > -50) strength = 4;       // Отличный сигнал
    else if (rssi > -60) strength = 3;  // Хороший сигнал
    else if (rssi > -70) strength = 2;  // Средний сигнал
    else if (rssi > -80) strength = 1;  // Слабый сигнал
    
    // Рисуем индикатор сигнала (4 полоски)
    for (int i = 0; i < 4; i++) {
        int barHeight = (i + 1);
        int barWidth = 1;
        int barX = x + i * 2;
        int barY = y + 4 - barHeight;
        
        if (i < strength) {
            // Активная полоса
            display->fillRect(barX, barY, barWidth, barHeight, color);
        } else {
            // Неактивная полоса
            display->drawRect(barX, barY, barWidth, barHeight, color);
        }
    }
}

// Функция для отображения индикатора уровня заряда батареи
void drawBattery(Adafruit_ST7735* display, int x, int y, int level, uint16_t color) {
    // Ограничиваем уровень от 0 до 100
    level = constrain(level, 0, 100);
    
    // Рисуем контур батареи
    display->drawRect(x, y, 10, 6, color);
    display->drawRect(x + 10, y + 1, 1, 4, color);
    
    // Рисуем заполнение в зависимости от уровня
    int fillWidth = 9 * level / 100;
    if (fillWidth > 0) {
        // Выбор цвета в зависимости от уровня
        uint16_t fillColor = COLOR_ERROR;  // Красный для низкого заряда
        if (level > 75) fillColor = COLOR_SUCCESS;  // Зеленый для высокого
        else if (level > 25) fillColor = COLOR_WARNING;  // Желтый для среднего
        
        display->fillRect(x + 1, y + 1, fillWidth, 4, fillColor);
    }
}

} // namespace DisplayUI