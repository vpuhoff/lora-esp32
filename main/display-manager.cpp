#include "display-manager.h"
#include "display-ui.h"
#include "statistics.h"
#include "lora-manager.h"
#include "wifi-manager.h"
#include "plot-manager.h"

// Глобальный экземпляр менеджера дисплея
extern DisplayManager* displayManager;

DisplayManager::DisplayManager(GyverDB* db) : _db(db) {
    _display = nullptr;
    _currentPage = PAGE_LOGO;
    _enabled = true;
    _brightness = 100;
    _autoScroll = true;
    _scrollInterval = 5000;
    _lastScrollTime = 0;
    _lastUpdateTime = 0;
    _needUpdate = true;
    _tempMessageTime = 0;
    _tempMessageDuration = 0;
    _isError = false;
}

DisplayManager::~DisplayManager() {
    if (_display != nullptr) {
        delete _display;
    }
}

bool DisplayManager::setupDisplay() {
    // Инициализация дисплея
    // Используем разные имена для пинов, чтобы избежать конфликта с существующими макросами
    #if DISPLAY_ENABLED
    _display = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RESET);
    
    if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(10000))) {
        // Инициализируем дисплей
        _display->initR(INITR_144GREENTAB); // Используем initR вместо begin

        // Настройка дисплея
        _display->setRotation(1);  // 0-3, поворот экрана
        _display->fillScreen(ST7735_BLACK);

        xSemaphoreGive(spi_lock_mutex);
        
        // Настройка подсветки, если используется управляемая подсветка
        #ifdef TFT_LED
        pinMode(TFT_LED, OUTPUT);
        setBrightness(_brightness);
        #endif
        
        logger.println(info_() + "Инициализация дисплея выполнена успешно");
        
        // Показываем стартовый экран
        showLogo();

    } else {
        logger.println(error_() + "Не удалось захватить мьютекс для инициализации дисплея");
        return false;
    }
    #endif
    return true;
}

void DisplayManager::initDefaults() {
    _db->init(DB_NAMESPACE::display_enabled, DISPLAY_DEFAULT_ENABLED);
    _db->init(DB_NAMESPACE::display_brightness, DISPLAY_DEFAULT_BRIGHTNESS);
    _db->init(DB_NAMESPACE::display_timeout, 60);  // 60 секунд тайм-аут
    _db->init(DB_NAMESPACE::display_auto_scroll, DISPLAY_DEFAULT_AUTO_SCROLL);
    _db->init(DB_NAMESPACE::display_scroll_interval, DISPLAY_DEFAULT_SCROLL_INTERVAL);
    
    // Загружаем настройки из БД
    _enabled = _db->get(DB_NAMESPACE::display_enabled).toBool();
    _brightness = _db->get(DB_NAMESPACE::display_brightness).toInt();
    _autoScroll = _db->get(DB_NAMESPACE::display_auto_scroll).toBool();
    _scrollInterval = _db->get(DB_NAMESPACE::display_scroll_interval).toInt();
}

void DisplayManager::applySettings() {
    _enabled = _db->get(DB_NAMESPACE::display_enabled).toBool();
    _brightness = _db->get(DB_NAMESPACE::display_brightness).toInt();
    _autoScroll = _db->get(DB_NAMESPACE::display_auto_scroll).toBool();
    _scrollInterval = _db->get(DB_NAMESPACE::display_scroll_interval).toInt();
    
    // Применяем яркость подсветки, если используется управляемая подсветка
    #ifdef TFT_LED
    setBrightness(_brightness);
    #endif
    
    // Если дисплей был отключен, очищаем его
    if (!_enabled && _display != nullptr) {
        _display->fillScreen(ST7735_BLACK);
        #ifdef TFT_LED
        analogWrite(TFT_LED, 0);  // Выключаем подсветку
        #endif
    } else if (_enabled && _display != nullptr) {
        _needUpdate = true;  // Пометка, что нужно обновить содержимое
    }
}

void DisplayManager::clear() {
    if (_display != nullptr && _enabled) {
        if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(1000))) {
            _display->fillScreen(ST7735_BLACK);
            xSemaphoreGive(spi_lock_mutex);
        } else {
            logger.println(error_() + "Не удалось захватить мьютекс для очистки дисплея");
        }
    }
}

void DisplayManager::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    
    #ifdef TFT_LED
    // Преобразование значения яркости (0-100) в шкалу ШИМ (0-255)
    uint8_t pwmValue = map(brightness, 0, 100, 0, 255);
    analogWrite(TFT_LED, pwmValue);
    #endif
}

void DisplayManager::enableDisplay(bool enable) {
    _enabled = enable;
    
    if (_display != nullptr) {
        if (!enable) {
            if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(1000))) {
                _display->fillScreen(ST7735_BLACK);
                xSemaphoreGive(spi_lock_mutex);
                #ifdef TFT_LED
                analogWrite(TFT_LED, 0);  // Выключаем подсветку
                #endif
            } else {
                logger.println(error_() + "Не удалось захватить мьютекс для выключения дисплея");
            }
        } else {
            _needUpdate = true;  // Пометка, что нужно обновить содержимое
            setBrightness(_brightness);  // Восстанавливаем яркость
        }
    }
}

bool DisplayManager::isEnabled() const {
    return _enabled;
}

void DisplayManager::nextPage() {
    if (_display != nullptr && _enabled) {
        _currentPage = static_cast<DisplayPage>((_currentPage + 1) % PAGE_COUNT);
        
        // Пропускаем страницу логов
        if (_currentPage == PAGE_LOGS) {
            _currentPage = static_cast<DisplayPage>((_currentPage + 1) % PAGE_COUNT);
        }
        
        _needUpdate = true;
    }
}

void DisplayManager::prevPage() {
    if (_display != nullptr && _enabled) {
        _currentPage = static_cast<DisplayPage>(_currentPage == 0 ? PAGE_COUNT - 1 : _currentPage - 1);
        
        // Пропускаем страницу логов
        if (_currentPage == PAGE_LOGS) {
            _currentPage = static_cast<DisplayPage>(_currentPage == 0 ? PAGE_COUNT - 1 : _currentPage - 1);
        }
        
        _needUpdate = true;
    }
}


void DisplayManager::setPage(DisplayPage pageIndex) {
    if (pageIndex >= 0 && pageIndex < PAGE_COUNT) {
        _currentPage = pageIndex;
        _needUpdate = true;
    }
}

DisplayPage DisplayManager::getCurrentPage() const {
    return _currentPage;
}

void DisplayManager::updateCurrentPage() {
    if (!_enabled || _display == nullptr) {
        return;
    }
    
    // Check if we need to show a temporary message
    if (_tempMessageTime > 0 && millis() - _tempMessageTime < _tempMessageDuration) {
        return;  // Continue showing temporary message
    } else if (_tempMessageTime > 0) {
        _tempMessageTime = 0;  // Reset temporary message
        _needUpdate = true;    // Force update of page
    }
    
    // Only update when needed or on a timer
    static uint32_t lastPartialUpdate = 0;
    
    // Check if we need a full update
    if (_needUpdate) {
        _needUpdate = false;
        _lastUpdateTime = millis();
        lastPartialUpdate = millis();
        
        if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(1000))) {
            // Clear screen before redrawing
            _display->fillScreen(ST7735_BLACK);
            
            // Draw page content based on the current page
            switch (_currentPage) {
                case PAGE_LOGO:
                    DisplayUI::drawLogoPage(_display);
                    break;
                case PAGE_LORA_STATUS:
                    DisplayUI::drawLoRaStatusPage(_display);
                    break;
                case PAGE_WIFI_STATUS:
                    DisplayUI::drawWiFiStatusPage(_display);
                    break;
                case PAGE_SYSTEM_INFO:
                    DisplayUI::drawSystemInfoPage(_display);
                    break;
                case PAGE_LOGS:
                    _currentPage = PAGE_SYSTEM_INFO;
                    _needUpdate = true;
                    xSemaphoreGive(spi_lock_mutex);
                    return;
            }
            
            // Draw page indicator and status bar
            DisplayUI::drawPageIndicator(_display, PAGE_COUNT, _currentPage);
            
            bool wifiConnected = WiFi.status() == WL_CONNECTED;
            bool loraActive = true;
            DisplayUI::drawStatusBar(_display, wifiConnected, loraActive);

            xSemaphoreGive(spi_lock_mutex);
        } else {
            logger.println(error_() + "Не удалось захватить мьютекс для обновления страницы");
            _needUpdate = true; // Попробуем обновить в следующий раз
            return;
        }
    }
    // Check if we need a partial update (just updating dynamic content)
    else if (millis() - lastPartialUpdate > 1000) {
        lastPartialUpdate = millis();
        
        // Only update dynamic parts of the current page
        // For example, just update time in status bar without redrawing everything
        switch (_currentPage) {
            case PAGE_SYSTEM_INFO:
                // Just update the CPU/memory values without redrawing the entire page
                // This would require changes to DisplayUI to support partial updates
                break;
            case PAGE_LORA_STATUS:
                // Update only the changing stats
                break;
        }
    }
    
    // Check for auto-scroll
    if (_autoScroll && millis() - _lastScrollTime > _scrollInterval) {
        _lastScrollTime = millis();
        nextPage();
    }
}


void DisplayManager::showInfo(String text, int duration) {
    if (!_enabled || _display == nullptr) {
        return;
    }
    
    _tempMessage = text;
    _tempMessageTime = millis();
    _tempMessageDuration = duration;
    _isError = false;
    
    if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(1000))) {
        _display->fillScreen(ST7735_BLACK);
        DisplayUI::drawInfoMessage(_display, _tempMessage);
        xSemaphoreGive(spi_lock_mutex);
    } else {
        logger.println(error_() + "Не удалось захватить мьютекс для отображения информации");
    }
}

void DisplayManager::showError(String text, int duration) {
    if (!_enabled || _display == nullptr) {
        return;
    }
    
    _tempMessage = text;
    _tempMessageTime = millis();
    _tempMessageDuration = duration;
    _isError = true;
    
    if (xSemaphoreTake(spi_lock_mutex, pdMS_TO_TICKS(1000))) {
        _display->fillScreen(ST7735_BLACK);
        DisplayUI::drawErrorMessage(_display, _tempMessage);
        xSemaphoreGive(spi_lock_mutex);
    } else {
        logger.println(error_() + "Не удалось захватить мьютекс для отображения ошибки");
    }
}

void DisplayManager::showLogo() {
    if (_enabled && _display != nullptr) {
        _currentPage = PAGE_LOGO;
        _needUpdate = true;
        updateCurrentPage();
    }
}

void DisplayManager::showLoRaStatus() {
    if (_enabled && _display != nullptr) {
        _currentPage = PAGE_LORA_STATUS;
        _needUpdate = true;
        updateCurrentPage();
    }
}

void DisplayManager::showWiFiStatus() {
    if (_enabled && _display != nullptr) {
        _currentPage = PAGE_WIFI_STATUS;
        _needUpdate = true;
        updateCurrentPage();
    }
}

void DisplayManager::showSystemInfo() {
    if (_enabled && _display != nullptr) {
        _currentPage = PAGE_SYSTEM_INFO;
        _needUpdate = true;
        updateCurrentPage();
    }
}

void DisplayManager::showLogs() {
    if (_enabled && _display != nullptr) {
        // Вместо показа страницы логов отображаем сообщение
        showInfo("Logs available in web interface", 2000);
        
        // НЕ устанавливаем текущую страницу как PAGE_LOGS
        // _currentPage = PAGE_LOGS;
        // _needUpdate = true;
    }
}

void DisplayManager::tick() {
    // Вызывается периодически для обработки событий дисплея
    if (!_enabled || _display == nullptr) {
        return;
    }
    
    // Обработка тайм-аута подсветки
    handleBacklight();
}

bool DisplayManager::checkAndResetUpdate() {
    bool result = _needUpdate;
    _needUpdate = false;
    return result;
}

void DisplayManager::handleBacklight() {
    static uint32_t lastActivityTime = millis();
    static bool backlightOff = false;
    
    // Получаем тайм-аут подсветки из настроек
    int timeout = _db->get(DB_NAMESPACE::display_timeout).toInt() * 1000;
    
    // Если тайм-аут равен 0, подсветка всегда включена
    if (timeout == 0) {
        if (backlightOff) {
            setBrightness(_brightness);
            backlightOff = false;
        }
        return;
    }
    
    // Проверяем активность (например, при смене страницы)
    if (_needUpdate) {
        lastActivityTime = millis();
        if (backlightOff) {
            setBrightness(_brightness);
            backlightOff = false;
        }
    }
    
    // Проверяем тайм-аут подсветки
    if (!backlightOff && millis() - lastActivityTime > timeout) {
        // Выключаем подсветку
        #ifdef TFT_LED
        analogWrite(TFT_LED, 0);
        #endif
        backlightOff = true;
    }
}