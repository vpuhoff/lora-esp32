#include "display-ui.h"
#include "statistics.h"
#include "plot-manager.h"
#include "system-monitor.h"
#include <vector>  // Для использования std::vector

namespace DisplayUI {

// Вспомогательная функция для разбивки строки на подстроки с учетом максимального количества символов в строке.
// Результат записывается в переданный вектор lines.
void splitText(const String &text, int maxChars, std::vector<String> &lines) {
    int start = 0;
    while (start < text.length()) {
        int end = start + maxChars;
        if (end >= text.length()) {
            lines.push_back(text.substring(start));
            break;
        }
        // Попытка найти последний пробел в пределах допустимой длины
        int spaceIndex = text.lastIndexOf(' ', end);
        if (spaceIndex < start) {
            spaceIndex = end; // если пробелов нет, отрезаем по maxChars
        }
        lines.push_back(text.substring(start, spaceIndex));
        start = spaceIndex + 1;
    }
}

// Функция для отрисовки прогресс-бара
void drawProgressBar(Adafruit_ST7735* display, int x, int y, int width, int height, int progress, uint16_t barColor, uint16_t bgColor) {
    progress = constrain(progress, 0, 100);
    // Отрисовка фона
    display->fillRect(x, y, width, height, bgColor);
    int progressWidth = (width * progress) / 100;
    if (progressWidth > 0) {
        display->fillRect(x, y, progressWidth, height, barColor);
    }
    // Отрисовка рамки
    display->drawRect(x, y, width, height, COLOR_TEXT);
}

// Оптимизированная функция отрисовки текстового блока с использованием пакетной печати строк
void drawTextBox(Adafruit_ST7735* display, int x, int y, int width, int height, String text, uint16_t borderColor, uint16_t textColor) {
    // Рисуем рамку текстового блока
    display->drawRect(x, y, width, height, borderColor);
    
    display->setTextColor(textColor);
    display->setTextSize(1);
    
    // Определяем внутренние границы для текста
    int innerX = x + 2;
    int innerY = y + 2;
    int innerWidth = width - 4;
    int innerHeight = height - 4;
    
    // Рассчитываем максимальное число символов в строке (средняя ширина символа ~6 пикселей)
    const int maxCharsPerLine = innerWidth / 6;
    std::vector<String> lines;
    splitText(text, maxCharsPerLine, lines);
    
    int lineHeight = 8;  // Высота строки при размере текста 1
    int currentY = innerY;
    for (size_t i = 0; i < lines.size(); i++) {
        if (currentY + lineHeight > innerY + innerHeight) break;
        display->setCursor(innerX, currentY);
        display->print(lines[i]);
        currentY += lineHeight;
    }
}

// Отрисовка индикатора страниц
void drawPageIndicator(Adafruit_ST7735* display, int totalPages, int currentPage, int yPosition) {
    int dotSize = 4;
    int spacing = 8;
    int totalWidth = totalPages * dotSize + (totalPages - 1) * spacing;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    for (int i = 0; i < totalPages; i++) {
        int x = startX + i * (dotSize + spacing);
        if (i == currentPage) {
            display->fillCircle(x + dotSize / 2, yPosition + dotSize / 2, dotSize / 2, COLOR_SUCCESS);
        } else {
            display->drawCircle(x + dotSize / 2, yPosition + dotSize / 2, dotSize / 2, COLOR_TEXT);
        }
    }
}

// Отрисовка статус-бара
void drawStatusBar(Adafruit_ST7735* display, bool wifiConnected, bool loraActive, int batteryLevel) {
    // Фон статус-бара
    display->fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_HEADER);
    
    // Индикатор WiFi
    if (wifiConnected) {
        drawWiFiSignal(display, 4, 2, WiFi.RSSI(), ST7735_WHITE);
    } else {
        display->drawLine(4, 2, 10, 8, ST7735_WHITE);
    }
    
    // Индикатор LoRa
    if (loraActive) {
        display->fillRect(15, 3, 3, 4, ST7735_WHITE);
        display->fillRect(19, 2, 3, 5, ST7735_WHITE);
        display->fillRect(23, 1, 3, 6, ST7735_WHITE);
    } else {
        display->drawRect(15, 3, 3, 4, 0x630C);
        display->drawRect(19, 2, 3, 5, 0x630C);
        display->drawRect(23, 1, 3, 6, 0x630C);
    }
    
    // Отрисовка уровня заряда батареи, если задан
    if (batteryLevel >= 0) {
        drawBattery(display, SCREEN_WIDTH - 15, 2, batteryLevel);
    }
    
    // Отрисовка времени работы
    display->setTextColor(ST7735_WHITE);
    display->setTextSize(1);
    display->setCursor((SCREEN_WIDTH / 2) - 15, 1);
    display->print(formatUptime(millis()));
}

// Отрисовка страницы логотипа и версии
void drawLogoPage(Adafruit_ST7735* display, const char* version) {
    drawHeader(display, "ESP32 LoRa");
    
    display->setTextSize(2);
    display->setTextColor(COLOR_SUCCESS);
    drawCenteredText(display, 40, "LoRa", COLOR_SUCCESS, 2);
    drawCenteredText(display, 60, "Control", COLOR_SUCCESS, 2);
    drawCenteredText(display, 80, "Panel", COLOR_SUCCESS, 2);
    
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT);
    drawCenteredText(display, 105, String("v") + version);
}

// Отрисовка страницы статуса LoRa
void drawLoRaStatusPage(Adafruit_ST7735* display) {
    drawHeader(display, "LoRa Status");
    
    if (loraManager == nullptr) {
        drawCenteredText(display, 70, "LoRa Manager", COLOR_TEXT, 1);
        drawCenteredText(display, 80, "not available", COLOR_ERROR, 1);
        return;
    }
    
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    display->setCursor(5, 25);
    display->print("SF: ");
    display->print(loraManager->getSpreadingFactor());
    
    display->setCursor(50, 25);
    display->print("BW: ");
    display->print(loraManager->getBandwidth());
    display->print(" kHz");
    
    display->setCursor(5, 35);
    display->print("CR: 4/");
    display->print(loraManager->getCodingRate());
    
    display->setCursor(50, 35);
    display->print("TX: ");
    display->print(loraManager->getTxPower());
    display->print(" dBm");
    
    display->setCursor(5, 50);
    display->print("Packets: ");
    display->print(loraManager->getPacketsTotal());
    
    display->setCursor(5, 60);
    display->print("Delivered: ");
    display->print(loraManager->getPacketsSuccess());
    
    display->setCursor(5, 70);
    display->print("Success rate: ");
    display->print(loraManager->getSuccessRate());
    display->print("%");
    
    drawProgressBar(display, 5, 95, SCREEN_WIDTH - 10, 10, loraManager->getSuccessRate());
}

// Отрисовка страницы статуса WiFi
void drawWiFiStatusPage(Adafruit_ST7735* display) {
    drawHeader(display, "WiFi Status");
    
    if (wifiManager == nullptr) {
        drawCenteredText(display, 60, "WiFi Manager", COLOR_TEXT, 1);
        drawCenteredText(display, 70, "not available", COLOR_ERROR, 1);
        return;
    }
    
    String wifiStatus = wifiManager->getStatusText();
    int startPos = 0;
    int lineNum = 1;
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
    
    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
        if (WiFi.status() == WL_CONNECTED) {
            int rssi = WiFi.RSSI();
            int signalStrength = map(constrain(rssi, -90, -30), -90, -30, 0, 100);
            
            display->setCursor(5, 65);
            display->print("Signal strength: ");
            display->print(signalStrength);
            display->print("%");
            
            uint16_t barColor = COLOR_ERROR;
            if (signalStrength > 70) barColor = COLOR_SUCCESS;
            else if (signalStrength > 30) barColor = COLOR_WARNING;
            
            drawProgressBar(display, 5, 75, SCREEN_WIDTH - 10, 10, signalStrength, barColor);
            
            display->setCursor(5, 90);
            display->print("RSSI: ");
            display->print(rssi);
            display->print(" dBm");
            
            int barCount = map(signalStrength, 0, 100, 0, 4);
            for (int i = 0; i < 4; i++) {
                int barHeight = (i + 1) * 5;
                int barX = (SCREEN_WIDTH / 2) - 20 + i * 10;
                int barY = 110 - barHeight;
                int barWidth = 6;
                
                if (i < barCount) {
                    display->fillRect(barX, barY, barWidth, barHeight, barColor);
                } else {
                    display->drawRect(barX, barY, barWidth, barHeight, COLOR_TEXT);
                }
            }
        }
    }
}

// Отрисовка страницы системной информации
void drawSystemInfoPage(Adafruit_ST7735* display) {
    drawHeader(display, "System Info");
    
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    display->setCursor(5, 25);
    display->print("Uptime: ");
    display->print(formatUptime(millis()));
    
    display->setCursor(5, 35);
    display->print("Free RAM: ");
    display->print(ESP.getFreeHeap() / 1024);
    display->print(" kB");
    
    display->setCursor(5, 45);
    display->print("Min Free: ");
    display->print(ESP.getMinFreeHeap() / 1024);
    display->print(" kB");
    
    if (systemMonitor != nullptr) {
        systemMonitor->update();
        
        display->setCursor(5, 60);
        display->print("CPU Usage: ");
        display->print(systemMonitor->getTotalCpuUsage());
        display->print("%");
        
        uint16_t cpuBarColor = COLOR_SUCCESS;
        uint32_t cpuUsage = systemMonitor->getTotalCpuUsage();
        if (cpuUsage > 80) cpuBarColor = COLOR_ERROR;
        else if (cpuUsage > 50) cpuBarColor = COLOR_WARNING;
        
        drawProgressBar(display, 5, 70, SCREEN_WIDTH - 10, 10, cpuUsage, cpuBarColor);
        
        display->setCursor(5, 85);
        display->print("Tasks:");
        
        uint16_t taskCount = 0;
        SystemMonitor::TaskInfo* tasks = systemMonitor->getTasksInfo(taskCount);
        
        if (tasks != nullptr && taskCount > 0) {
            int y = 95;
            for (uint16_t i = 0; i < taskCount && i < 3; i++) {
                if (taskCount > 1 && (strcmp(tasks[i].name, "IDLE") == 0 ||
                    strncmp(tasks[i].name, "tiT", 3) == 0)) {
                    continue;
                }
                display->setCursor(5, y);
                display->print(tasks[i].name);
                
                display->setCursor(80, y);
                display->print(tasks[i].cpuUsage);
                display->print("%");
                
                y += 10;
                if (y > 125) break;
            }
            vPortFree(tasks);
        } else {
            display->setCursor(5, 95);
            display->print("Task data unavailable");
        }
    } else {
        display->setCursor(5, 60);
        display->print("CPU monitoring not available");
    }
}

// Отрисовка страницы логов
void drawLogsPage(Adafruit_ST7735* display, const String* logLines, int lineCount) {
    drawHeader(display, "System Logs");
    
    if (lineCount == 0) {
        drawCenteredText(display, 60, "No logs available");
        return;
    }
    
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    for (int i = 0; i < lineCount && i < 10; i++) {
        String line = logLines[i];
        uint16_t color = COLOR_TEXT;
        if (line.startsWith("err:") || line.startsWith("error:")) {
            color = COLOR_ERROR;
        } else if (line.startsWith("warn:") || line.startsWith("warning:")) {
            color = COLOR_WARNING;
        } else if (line.startsWith("info:")) {
            color = COLOR_SUCCESS;
        }
        if (line.length() > 21) {
            line = line.substring(0, 19) + "..";
        }
        display->setTextColor(color);
        display->setCursor(5, 15 + i * 10);
        display->print(line);
    }
}

// Отрисовка страницы мониторинга CPU
void drawCpuMonitorPage(Adafruit_ST7735* display) {
    drawHeader(display, "CPU Monitor");
    
    if (systemMonitor == nullptr) {
        drawCenteredText(display, 64, "CPU Monitor", COLOR_TEXT);
        drawCenteredText(display, 84, "Not available", COLOR_ERROR);
        return;
    }
    
    systemMonitor->update();
    
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT);
    display->setCursor(5, 15);
    display->print("CPU: ");
    display->print(systemMonitor->getTotalCpuUsage());
    display->print("% Free: ");
    display->print(systemMonitor->getFreeHeap() / 1024);
    display->print("kB");
    
    display->drawLine(0, 25, 128, 25, COLOR_HEADER);
    
    uint16_t taskCount = 0;
    SystemMonitor::TaskInfo* tasks = systemMonitor->getTasksInfo(taskCount);
    
    if (tasks != nullptr && taskCount > 0) {
        uint8_t displayLimit = min(6, (int)taskCount);
        
        display->setCursor(5, 30);
        display->println("Task      CPU  Stack");
        
        int y = 40;
        for (uint8_t i = 0; i < displayLimit; i++) {
            if (taskCount > 6 && (strcmp(tasks[i].name, "IDLE") == 0 || strncmp(tasks[i].name, "tiT", 3) == 0)) {
                continue;
            }
            display->setCursor(5, y);
            char shortName[9];
            strncpy(shortName, tasks[i].name, 8);
            shortName[8] = '\0';
            display->print(shortName);
            
            display->setCursor(70, y);
            display->print(tasks[i].cpuUsage);
            display->print("%");
            
            display->setCursor(95, y);
            display->print(tasks[i].stackHighWater);
            
            uint16_t barColor;
            if (tasks[i].cpuUsage > 50) barColor = COLOR_ERROR;
            else if (tasks[i].cpuUsage > 20) barColor = COLOR_WARNING;
            else barColor = COLOR_SUCCESS;
            
            int barWidth = map(tasks[i].cpuUsage, 0, 100, 0, 60);
            display->drawRect(0, y, 3, 8, barColor);
            if (barWidth > 0) {
                display->fillRect(0, y, 3, 8, barColor);
            }
            
            y += 12;
            if (y > 110) break;
        }
        vPortFree(tasks);
    } else {
        display->setCursor(5, 50);
        display->println("No task data available");
    }
}

// Оптимизированная функция отрисовки информационного сообщения с разбивкой текста на строки
void drawInfoMessage(Adafruit_ST7735* display, String message) {
    drawHeader(display, "Information");
    int boxWidth = SCREEN_WIDTH - 20;
    int boxHeight = 60;
    int boxX = 10;
    int boxY = 30;
    
    display->fillRect(boxX, boxY, boxWidth, boxHeight, COLOR_HEADER);
    display->drawRect(boxX, boxY, boxWidth, boxHeight, COLOR_TEXT);
    
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    const int maxCharsPerLine = (boxWidth - 4) / 6;
    std::vector<String> lines;
    splitText(message, maxCharsPerLine, lines);
    
    int currentY = boxY + 10;
    int lineHeight = 10; // Немного увеличенная высота для лучшей читаемости
    for (size_t i = 0; i < lines.size(); i++) {
        if (currentY + lineHeight > boxY + boxHeight - 10) break;
        drawCenteredText(display, currentY, lines[i]);
        currentY += lineHeight;
    }
}

// Оптимизированная функция отрисовки сообщения об ошибке с разбивкой текста
void drawErrorMessage(Adafruit_ST7735* display, String message) {
    drawHeader(display, "Error", COLOR_ERROR);
    int boxWidth = SCREEN_WIDTH - 20;
    int boxHeight = 60;
    int boxX = 10;
    int boxY = 30;
    
    display->fillRect(boxX, boxY, boxWidth, boxHeight, 0x630C);
    display->drawRect(boxX, boxY, boxWidth, boxHeight, COLOR_ERROR);
    
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    
    const int maxCharsPerLine = (boxWidth - 4) / 6;
    std::vector<String> lines;
    splitText(message, maxCharsPerLine, lines);
    
    int currentY = boxY + 10;
    int lineHeight = 10;
    for (size_t i = 0; i < lines.size(); i++) {
        if (currentY + lineHeight > boxY + boxHeight - 10) break;
        drawCenteredText(display, currentY, lines[i]);
        currentY += lineHeight;
    }
}

// Вспомогательная функция для отрисовки центрированного текста
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

// Функция для отрисовки заголовка
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
    uint32_t maxVal = 0;
    uint32_t minVal = UINT32_MAX;
    
    for (int i = 0; i < dataPoints; i++) {
        if (data[i] > maxVal) maxVal = data[i];
        if (data[i] < minVal) minVal = data[i];
    }
    
    if (maxVal == minVal) {
        maxVal += 10;
        minVal = (minVal > 10) ? minVal - 10 : 0;
    }
    
    for (int i = 1; i < dataPoints; i++) {
        int x1 = x + (width * (i - 1)) / dataPoints;
        int y1 = y + height - ((height * (data[i - 1] - minVal)) / (maxVal - minVal));
        
        int x2 = x + (width * i) / dataPoints;
        int y2 = y + height - ((height * (data[i] - minVal)) / (maxVal - minVal));
        
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

// Функция для отрисовки индикатора сигнала WiFi (с 0 до 4 полос)
void drawWiFiSignal(Adafruit_ST7735* display, int x, int y, int rssi, uint16_t color) {
    int strength = 0;
    if (rssi > -50) strength = 4;
    else if (rssi > -60) strength = 3;
    else if (rssi > -70) strength = 2;
    else if (rssi > -80) strength = 1;
    
    for (int i = 0; i < 4; i++) {
        int barHeight = (i + 1);
        int barWidth = 1;
        int barX = x + i * 2;
        int barY = y + 4 - barHeight;
        
        if (i < strength) {
            display->fillRect(barX, barY, barWidth, barHeight, color);
        } else {
            display->drawRect(barX, barY, barWidth, barHeight, color);
        }
    }
}

// Функция для отрисовки индикатора уровня заряда батареи
void drawBattery(Adafruit_ST7735* display, int x, int y, int level, uint16_t color) {
    level = constrain(level, 0, 100);
    
    display->drawRect(x, y, 10, 6, color);
    display->drawRect(x + 10, y + 1, 1, 4, color);
    
    int fillWidth = 9 * level / 100;
    if (fillWidth > 0) {
        uint16_t fillColor = COLOR_ERROR;
        if (level > 75) fillColor = COLOR_SUCCESS;
        else if (level > 25) fillColor = COLOR_WARNING;
        
        display->fillRect(x + 1, y + 1, fillWidth, 4, fillColor);
    }
}

} // namespace DisplayUI
