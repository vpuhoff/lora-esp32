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
    // Статические переменные для хранения настроек WiFi.
    static bool wifiInit = false;
    static int currentWifiModePersistent = -1;
    static String currentApSSID = "";
    static String currentApPass = "";
    static String currentStaSSID = "";
    static String currentStaPass = "";
    if (!wifiInit) {
        currentWifiModePersistent = _db->get(DB_NAMESPACE::wifi_mode).toInt();
        currentApSSID = _db->get(DB_NAMESPACE::ap_ssid);
        currentApPass = _db->get(DB_NAMESPACE::ap_pass);
        currentStaSSID = _db->get(DB_NAMESPACE::sta_ssid);
        currentStaPass = _db->get(DB_NAMESPACE::sta_pass);
        wifiInit = true;
    }
    {
        sets::Group g(b, "Настройки WiFi");
        
        // Читаем текущее значение режима WiFi один раз
        if (currentWifiModePersistent == -1) {
            currentWifiModePersistent = _db->get(DB_NAMESPACE::wifi_mode).toInt();
        }

        // Виджет Select, который сохраняет выбранное значение в currentWifiModePersistent
        b.Select("Режим работы", "Только AP;Только STA;AP + STA", &currentWifiModePersistent);
        Serial.println("Текущий режим WiFi (persistent): " + String(currentWifiModePersistent));
        
        // Группа настроек для точки доступа (AP)
        if (currentWifiModePersistent == MY_WIFI_MODE_AP || currentWifiModePersistent == MY_WIFI_MODE_AP_STA) {
            Serial.println("Отображаем настройки точки доступа");
            b.Label(H("ap_label"), "Настройки точки доступа:");
            b.Input(H("ap_ssid"), "SSID точки доступа", &currentApSSID);
            b.Pass("Пароль точки доступа", &currentApPass);
            b.reload();
        } else {
            Serial.println("Настройки точки доступа не отображаются");
        }
        
        // Группа настроек для подключения (STA)
        if (currentWifiModePersistent == MY_WIFI_MODE_STA || currentWifiModePersistent == MY_WIFI_MODE_AP_STA) {
            Serial.println("Отображаем настройки подключения к WiFi");
            b.Label(H("sta_label"), "Настройки подключения к WiFi:");
            b.Input(H("sta_ssid"), "SSID для подключения", &currentStaSSID);
            b.Pass("Пароль для подключения", &currentStaPass);
            b.reload();
        } else {
            Serial.println("Настройки подключения не отображаются");
        }
        
        // Кнопка применения настроек
        if (b.Button(DB_NAMESPACE::apply_wifi, "Применить настройки WiFi")) {
            Serial.println("Применяем настройки WiFi, новый режим: " + String(currentWifiModePersistent));
            _db->update(DB_NAMESPACE::wifi_mode, currentWifiModePersistent);
            _db->update(DB_NAMESPACE::ap_ssid, currentApSSID);
            _db->update(DB_NAMESPACE::ap_pass, currentApPass);
            _db->update(DB_NAMESPACE::sta_ssid, currentStaSSID);
            _db->update(DB_NAMESPACE::sta_pass, currentStaPass);
            _needRestart = true; // Отметка о необходимости перезагрузки
        }
    }
    
    // Настройки LoRa
    // Статические переменные для хранения настроек LoRa.
    static bool loraInit = false;
    static int currentLoraSpreading = 0;
    static String currentLoraBandwidth = "";
    static int currentLoraCodingRate = 0;
    static int currentLoraMaxAttempts = 0;
    static int currentLoraTxPower = 0;
    if (!loraInit) {
        currentLoraSpreading = _db->get(DB_NAMESPACE::lora_spreading).toInt();
        currentLoraBandwidth = _db->get(DB_NAMESPACE::lora_bandwidth).toString();
        currentLoraCodingRate = _db->get(DB_NAMESPACE::lora_coding_rate).toInt();
        currentLoraMaxAttempts = _db->get(DB_NAMESPACE::lora_max_attempts).toInt();
        currentLoraTxPower = _db->get(DB_NAMESPACE::lora_tx_power).toInt();
        loraInit = true;
    }
    {
        sets::Group g(b, "Настройки LoRa");
        
        String sfOptions = "7;8;9;10;11;12";
        b.Select("Spreading Factor", sfOptions, &currentLoraSpreading);
        
        String bwOptions = "7.8;10.4;15.6;20.8;31.25;41.7;62.5;125;250;500";
        b.Select("Bandwidth (kHz)", bwOptions, &currentLoraBandwidth);
        
        String crOptions = "5;6;7;8";
        b.Select("Coding Rate (4/x)", crOptions, &currentLoraCodingRate);
        
        b.Slider(H("lora_max_attempts"), "Макс. число попыток", 1.0f, 10.0f, 1.0f, "", &currentLoraMaxAttempts);
        b.Slider(H("lora_tx_power"), "Мощность передачи (dBm)", 2.0f, 20.0f, 1.0f, "", &currentLoraTxPower);
    
        if (b.Button(H("apply_lora"), "Применить настройки LoRa")) {
            logger.add(String("Spreading:") + currentLoraSpreading);
            logger.add(String("Bandwidth:") + currentLoraBandwidth.toFloat());
            logger.add(String("CodingRate:") + currentLoraCodingRate);
            logger.add(String("TxPower:") + currentLoraTxPower);
            _db->set(DB_NAMESPACE::lora_spreading, currentLoraSpreading);
            _db->set(DB_NAMESPACE::lora_bandwidth, currentLoraBandwidth.toFloat());
            _db->set(DB_NAMESPACE::lora_coding_rate, currentLoraCodingRate);
            _db->set(DB_NAMESPACE::lora_max_attempts, currentLoraMaxAttempts);
            _db->set(DB_NAMESPACE::lora_tx_power, currentLoraTxPower);
            loraManager->applySettings();
            _needRestart = true; // Отметка о необходимости перезагрузки
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
