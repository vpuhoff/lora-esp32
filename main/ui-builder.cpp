#include "ui-builder.h"
#include "statistics.h"

// Создаём статический экземпляр логгера для виджета Log с размером буфера 10000 байт.
static sets::Logger logWidget(10000);

// Конструктор
UIBuilder::UIBuilder(GyverDB* db) : _db(db), _needRestart(false) {
}

// Главная функция для построения интерфейса
void UIBuilder::buildInterface(sets::Builder& b) {
    enum Tabs : uint8_t {
        Dashboard,
        LoRaStatus,
        Logs,
        Settings
    } static tab;

    if (b.Tabs("Dashboard;LoRa Status;Logs;Settings", (uint8_t*)&tab)) {
        b.reload();
        return;
    }

    switch (tab) {
        case Tabs::Dashboard:
            buildDashboardTab(b);
            break;
        case Tabs::LoRaStatus:
            buildLoRaStatusTab(b);
            break;
        case Tabs::Logs:
            buildLogsTab(b);
            break;
        case Tabs::Settings:
            buildSettingsTab(b);
            break;
    }
}

// Функция для отображения вкладки с панелью мониторинга
void UIBuilder::buildDashboardTab(sets::Builder& b) {
    // Информация о состоянии WiFi
    {
        sets::Group g(b, "Статус системы");
        b.Label(wifiManager->getStatusText());
        b.Label("Время работы: " + String(millis() / 1000) + " сек");
        b.Label("Свободная память: " + String(ESP.getFreeHeap()) + " байт");
    }
    
    // Статистика LoRa
    {
        sets::Group g(b, "Общая статистика LoRa");
        b.Label("Отправлено пакетов: " + String(totalSent));
        b.Label("Доставлено пакетов: " + String(totalReceived));
        if (totalSent > 0) {
            float successRate = (totalReceived / (float)totalSent) * 100.0;
            b.Label("Успешность доставки: " + String(successRate, 1) + "%");
            b.Label("Сглаженная успешность: " + String(successRateSmoothed, 1) + "%");
        }
    }
    
    // График данных
    {
        sets::Group g(b, "График успешности доставки");
        // Вызываем Plot с тремя параметрами: идентификатор, JSON-данные и объединённая подпись осей.
        b.Plot(H(plot), plotManager.getPlotJson(), "Время, Успешность (%)");
    }
}

// Функция отображения вкладки с логами
void UIBuilder::buildLogsTab(sets::Builder& b) {
    // Настройки логгера
    {
        sets::Group g(b, "Настройки логгера");
        b.Select(DB_NAMESPACE::log_level, "Уровень вывода логов", "INFO,WARN,ERROR,DEBUG");
        if (b.Button(DB_NAMESPACE::clear_log, "Очистить лог")) {
            logWidget.clear();
            // Вызов logWidget.add("Лог очищен") удалён, так как метод add отсутствует.
            // При необходимости можно добавить сообщение в глобальный лог с помощью другого объекта.
        }
        // Обновление уровня логирования можно реализовать по необходимости.
    }
    
    // Вывод логов с использованием виджета Log
    {
        sets::Group g(b, "Логи системы");
        b.Log(logWidget, "Логи системы");
    }
}

// Функция отображения статуса LoRa
void UIBuilder::buildLoRaStatusTab(sets::Builder& b) {
    loraManager->updateStats();
    {
        sets::Group g(b, "Текущие настройки LoRa");
        b.Label("Spreading Factor: SF" + String(loraManager->getSpreadingFactor()));
        b.Label("Bandwidth: " + String(loraManager->getBandwidth()) + " kHz");
        b.Label("Coding Rate: 4/" + String(loraManager->getCodingRate()));
        b.Label("Максимум попыток: " + String(loraManager->getMaxAttempts()));
        b.Label("Мощность передачи: " + String(loraManager->getTxPower()) + " dBm");
    }
    {
        sets::Group g(b, "Статистика передачи");
        b.Label("Всего пакетов: " + String(loraManager->getPacketsTotal()));
        b.Label("Успешно доставлено: " + String(loraManager->getPacketsSuccess()));
        if (loraManager->getPacketsTotal() > 0) {
            b.Label("Успешность доставки: " + String(loraManager->getSuccessRate()) + "%");
            b.Label("Сглаженная успешность: " + String(successRateSmoothed, 1) + "%");
        }
        b.Label("Последний RSSI: " + String(loraManager->getLastRssi(), 1) + " dBm");
    }
}

// Функция отображения вкладки с настройками
void UIBuilder::buildSettingsTab(sets::Builder& b) {
    // Настройки WiFi
    {
        sets::Group g(b, "Настройки WiFi");
        
        // Выбор режима WiFi (опции разделены точкой с запятой)
        b.Select(DB_NAMESPACE::wifi_mode, "Режим работы", "Только AP;Только STA;AP + STA");
        
        // Настройки точки доступа – если выбран режим AP или комбинированный режим AP+STA
        if (_db->get(DB_NAMESPACE::wifi_mode).toInt() == MY_WIFI_MODE_AP ||
            _db->get(DB_NAMESPACE::wifi_mode).toInt() == MY_WIFI_MODE_AP_STA) {
            sets::Group ap(b, "Настройки точки доступа");
            b.Input(DB_NAMESPACE::ap_ssid, "SSID точки доступа");
            b.Pass(DB_NAMESPACE::ap_pass, "Пароль точки доступа", "******");
        }
        
        // Настройки подключения к WiFi – если выбран режим STA или комбинированный режим AP+STA
        if (_db->get(DB_NAMESPACE::wifi_mode).toInt() == MY_WIFI_MODE_STA ||
            _db->get(DB_NAMESPACE::wifi_mode).toInt() == MY_WIFI_MODE_AP_STA) {
            sets::Group sta(b, "Настройки подключения к WiFi");
            b.Input(DB_NAMESPACE::sta_ssid, "SSID для подключения");
            b.Pass(DB_NAMESPACE::sta_pass, "Пароль для подключения", "******");
        }
        
        // Применение настроек
        if (b.Button(DB_NAMESPACE::apply_wifi, "Применить настройки WiFi")) {
            _db->update(DB_NAMESPACE::wifi_mode, _db->get(DB_NAMESPACE::wifi_mode));
            _db->update(DB_NAMESPACE::ap_ssid, _db->get(DB_NAMESPACE::ap_ssid));
            _db->update(DB_NAMESPACE::ap_pass, _db->get(DB_NAMESPACE::ap_pass));
            _db->update(DB_NAMESPACE::sta_ssid, _db->get(DB_NAMESPACE::sta_ssid));
            _db->update(DB_NAMESPACE::sta_pass, _db->get(DB_NAMESPACE::sta_pass));
            _needRestart = true; // Помечаем, что нужна перезагрузка
        }
    }
    
    // Настройки LoRa
    {
        sets::Group g(b, "Настройки LoRa");
        
        // Spreading Factor (SF) – список опций через точку с запятой
        String sfOptions = "7;8;9;10;11;12";
        b.Select(DB_NAMESPACE::lora_spreading, "Spreading Factor", sfOptions);
        
        // Bandwidth (BW)
        String bwOptions = "7.8;10.4;15.6;20.8;31.25;41.7;62.5;125;250;500";
        b.Select(DB_NAMESPACE::lora_bandwidth, "Bandwidth (kHz)", bwOptions);
        
        // Coding Rate (CR)
        String crOptions = "5;6;7;8";
        b.Select(DB_NAMESPACE::lora_coding_rate, "Coding Rate (4/x)", crOptions);
        
        // Максимальное количество попыток
        b.Slider(DB_NAMESPACE::lora_max_attempts, "Макс. число попыток", 1, 10, 1);
        
        // Мощность передачи
        b.Slider(DB_NAMESPACE::lora_tx_power, "Мощность передачи (dBm)", 2, 20, 1);
        
        // Кнопка применения настроек LoRa
        if (b.Button(DB_NAMESPACE::apply_lora, "Применить настройки LoRa")) {
            loraManager->applySettings();
        }
    }
    
    // Управление устройством
    {
        sets::Group g(b, "Управление устройством");
        if (b.Button(DB_NAMESPACE::restart_device, "Перезагрузить устройство")) {
            logger.add("Запрос на перезагрузку устройства...");
            _needRestart = true;
        }
    }
}


// Проверка необходимости перезагрузки
bool UIBuilder::needsRestart() const {
    return _needRestart;
}
