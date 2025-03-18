#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"
#include "system-monitor.h"
#include "statistics.h"
#include "wifi-manager.h"
#include "lora-manager.h"

namespace DisplayUI {
    // Константы для UI
    constexpr int SCREEN_WIDTH = 128;
    constexpr int SCREEN_HEIGHT = 128;
    constexpr int STATUS_BAR_HEIGHT = 10;
    constexpr int PAGE_INDICATOR_HEIGHT = 6;
    
    // Цвета для интерфейса
    constexpr uint16_t COLOR_BACKGROUND = ST77XX_BLACK;
    constexpr uint16_t COLOR_TEXT = ST77XX_WHITE;
    constexpr uint16_t COLOR_HEADER = ST77XX_BLUE;
    constexpr uint16_t COLOR_SUCCESS = ST77XX_GREEN;
    constexpr uint16_t COLOR_WARNING = ST77XX_YELLOW;
    constexpr uint16_t COLOR_ERROR = ST77XX_RED;
    constexpr uint16_t COLOR_PROGRESS_BAR = ST77XX_CYAN;
    constexpr uint16_t COLOR_PROGRESS_BG = 0x528A;    // Темно-серый
    
    // Функции для отрисовки элементов интерфейса
    void drawProgressBar(Adafruit_ST7735* display, int x, int y, int width, int height, int progress, uint16_t barColor = COLOR_PROGRESS_BAR, uint16_t bgColor = COLOR_PROGRESS_BG);
    
    void drawTextBox(Adafruit_ST7735* display, int x, int y, int width, int height, String text, uint16_t borderColor = COLOR_TEXT, uint16_t textColor = COLOR_TEXT);
    
    void drawPageIndicator(Adafruit_ST7735* display, int totalPages, int currentPage, int yPosition = SCREEN_HEIGHT - PAGE_INDICATOR_HEIGHT - 1);
    
    void drawStatusBar(Adafruit_ST7735* display, bool wifiConnected, bool loraActive, int batteryLevel = -1);
    
    // Функции для отрисовки страниц
    void drawLogoPage(Adafruit_ST7735* display, const char* version = "1.0");
    
    void drawLoRaStatusPage(Adafruit_ST7735* display);
    
    void drawWiFiStatusPage(Adafruit_ST7735* display);
    
    void drawSystemInfoPage(Adafruit_ST7735* display);
    
    void drawLogsPage(Adafruit_ST7735* display, const String* logLines, int lineCount);
    
    void drawCpuMonitorPage(Adafruit_ST7735* display);
    
    // Функции для отображения сообщений
    void drawInfoMessage(Adafruit_ST7735* display, String message);
    
    void drawErrorMessage(Adafruit_ST7735* display, String message);
    
    // Вспомогательные функции
    void drawCenteredText(Adafruit_ST7735* display, int y, String text, uint16_t color = COLOR_TEXT, uint8_t size = 1);
    
    void drawHeader(Adafruit_ST7735* display, String title, uint16_t color = COLOR_HEADER);
    
    // Функция для отрисовки миниграфика
    void drawMiniGraph(Adafruit_ST7735* display, int x, int y, int width, int height, const uint32_t* data, int dataPoints, uint16_t lineColor = COLOR_SUCCESS);
    
    // Функция для форматирования IP-адреса
    String formatIP(IPAddress ip);
    
    // Функция для форматирования времени работы
    String formatUptime(uint32_t milliseconds);
    
    // Функция для отображения индикатора сигнала WiFi (0-4 полоски)
    void drawWiFiSignal(Adafruit_ST7735* display, int x, int y, int strength, uint16_t color = COLOR_TEXT);
    
    // Функция для отображения индикатора уровня заряда батареи
    void drawBattery(Adafruit_ST7735* display, int x, int y, int level, uint16_t color = COLOR_TEXT);
}