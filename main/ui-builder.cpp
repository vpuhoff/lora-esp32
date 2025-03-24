#include "ui-builder.h"
#include "statistics.h"
#include "logging.h"

static String sfOptions = "7;8;9;10;11;12";
static int sfOptionsValues[] = {7, 8, 9, 10, 11, 12};
static String bwOptions = "7.8;10.4;15.6;20.8;31.25;41.7;62.5;125;250;500";
static float bwOptionsValues[] = {7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500};
static String crOptions = "5;6;7;8";
static int crOptionsValues[] = {5, 6, 7, 8};

// Конструктор
UIBuilder::UIBuilder(GyverDB* db) : _db(db), _needRestart(false) {
}

// Главная функция для построения интерфейса
void UIBuilder::buildInterface(sets::Builder& b) {
    enum Tabs : uint8_t {
        Dashboard,
        LoRaStatus,
        Logs,
        Settings,
        #if DISPLAY_ENABLED
        Display,     // Вкладка дисплея только для ESP32
        #endif
        SystemMonitor // Добавляем вкладку системного мониторинга
    } static tab;

    #if DISPLAY_ENABLED
    if (b.Tabs("Dashboard;LoRa Status;Logs;Settings;Display;System", (uint8_t*)&tab)) {
    #else
    if (b.Tabs("Dashboard;LoRa Status;Logs;Settings;System", (uint8_t*)&tab)) {
    #endif
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
        #if DISPLAY_ENABLED
        case Tabs::Display:
            buildDisplayTab(b);
            break;
        #endif
        case Tabs::SystemMonitor: // Обработка вкладки мониторинга
            buildSystemMonitorTab(b);
            break;
    }
}

// Добавить реализацию метода buildDisplayTab:
void UIBuilder::buildDisplayTab(sets::Builder& b) {
    // Статические переменные для настроек дисплея
    static bool displayInit = false;
    static bool displayEnabled = true;
    static int displayBrightness = 100;
    static int displayTimeout = 60;
    static bool displayAutoScroll = true;
    static int displayScrollInterval = 5;

    if (!displayInit) {
        displayEnabled = _db->get(DB_NAMESPACE::display_enabled).toBool();
        displayBrightness = _db->get(DB_NAMESPACE::display_brightness).toInt();
        displayTimeout = _db->get(DB_NAMESPACE::display_timeout).toInt();
        displayAutoScroll = _db->get(DB_NAMESPACE::display_auto_scroll).toBool();
        displayScrollInterval = _db->get(DB_NAMESPACE::display_scroll_interval).toInt() / 1000;
        displayInit = true;
    }

    {
        sets::Group g(b, "Настройки дисплея");

        b.Switch(H("display_enabled"), "Включить дисплей", &displayEnabled);
        b.Slider(H("display_brightness"), "Яркость дисплея", 0, 100, 5, "%");
        b.Slider(H("display_timeout"), "Тайм-аут подсветки (сек)", 0, 300, 10, "сек");
        b.Switch(H("display_auto_scroll"), "Автоматическое переключение страниц", &displayAutoScroll);

        if (displayAutoScroll) {
            b.Slider(H("display_scroll_interval"), "Интервал переключения (сек)", 1, 30, 1, "сек");
        }

        if (b.Button(H("apply_display"), "Применить настройки дисплея")) {
            _db->update(DB_NAMESPACE::display_enabled, displayEnabled);
            _db->update(DB_NAMESPACE::display_brightness, displayBrightness);
            _db->update(DB_NAMESPACE::display_timeout, displayTimeout);
            _db->update(DB_NAMESPACE::display_auto_scroll, displayAutoScroll);
            _db->update(DB_NAMESPACE::display_scroll_interval, displayScrollInterval * 1000);

            if (displayManager) {
                displayManager->applySettings();
            }
        }
    }

    {
        sets::Group g(b, "Управление дисплеем");

        if (b.Button(H("display_next"), "Следующая страница")) {
            if (displayManager) {
                displayManager->nextPage();
            }
        }

        if (b.Button(H("display_show_logo"), "Показать логотип")) {
            if (displayManager) {
                displayManager->showLogo();
            }
        }

        if (b.Button(H("display_show_lora"), "Показать статус LoRa")) {
            if (displayManager) {
                displayManager->showLoRaStatus();
            }
        }

        if (b.Button(H("display_show_wifi"), "Показать статус WiFi")) {
            if (displayManager) {
                displayManager->showWiFiStatus();
            }
        }

        if (b.Button(H("display_show_system"), "Показать системную информацию")) {
            if (displayManager) {
                displayManager->showSystemInfo();
            }
        }
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

    // // График данных
    // {
    //     sets::Group g(b, "График успешности доставки");
    //     // Вызываем Plot с тремя параметрами: идентификатор, JSON-данные и объединённая подпись осей.
    //     b.Plot(H(plot), plotManager.getPlotJson(), "Время, Успешность (%)");
    // }
}

// Функция отображения вкладки с логами
void UIBuilder::buildLogsTab(sets::Builder& b) {
    // Вывод логов с использованием виджета Log
    {
        sets::Group g(b, "Логи системы");
        b.Log(H("logger"), logger);
    }
    if (b.Button("Test Log")) {
        logger.println(millis());
        logger.println("info: This is an info message");
        logger.println(warn_() + "This is a warning message");
        logger.println("err: This is an error message");
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
        // b.Label("Последний RSSI: " + String(loraManager->getLastRssi(), 1) + " dBm");
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
        logger.println("Текущий режим WiFi (persistent): " + String(currentWifiModePersistent));

        // Группа настроек для точки доступа (AP)
        if (currentWifiModePersistent == MY_WIFI_MODE_AP || currentWifiModePersistent == MY_WIFI_MODE_AP_STA) {
            logger.println("Отображаем настройки точки доступа");
            b.Label(H("ap_label"), "Настройки точки доступа:");
            b.Input(H("ap_ssid"), "SSID точки доступа", &currentApSSID);
            b.Pass("Пароль точки доступа", &currentApPass);
            b.reload();
        } else {
            logger.println("Настройки точки доступа не отображаются");
        }

        // Группа настроек для подключения (STA)
        if (currentWifiModePersistent == MY_WIFI_MODE_STA || currentWifiModePersistent == MY_WIFI_MODE_AP_STA) {
            logger.println("Отображаем настройки подключения к WiFi");
            b.Label(H("sta_label"), "Настройки подключения к WiFi:");
            b.Input(H("sta_ssid"), "SSID для подключения", &currentStaSSID);
            b.Pass("Пароль для подключения", &currentStaPass);
            b.reload();
        } else {
            logger.println("Настройки подключения не отображаются");
        }

        // Кнопка применения настроек
        if (b.Button(DB_NAMESPACE::apply_wifi, "Применить настройки WiFi")) {
            logger.println("Применяем настройки WiFi, новый режим: " + String(currentWifiModePersistent));
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

        b.Select(DB_NAMESPACE::lora_spreading_selected, "Spreading Factor", sfOptions);
        b.Select(DB_NAMESPACE::lora_bandwidth_selected, "Bandwidth (kHz)", bwOptions);
        b.Select(DB_NAMESPACE::lora_coding_rate_selected, "Coding Rate (4/x)", crOptions);
        b.Slider(DB_NAMESPACE::lora_max_attempts, "Макс. число попыток", 1.0f, 10.0f, 1.0f, "");
        b.Slider(DB_NAMESPACE::lora_tx_power, "Мощность передачи (dBm)", 2.0f, 20.0f, 1.0f, "");

        // обработка действий
        switch (b.build.id) {
            case DB_NAMESPACE::lora_spreading_selected:
                logger.println(String("current_lora_spreading:") + String(sfOptions[b.build.value]));
                currentLoraSpreading = String(sfOptions[b.build.value]).toInt();
                break;
            case DB_NAMESPACE::lora_bandwidth_selected:
                logger.println(String("current_lora_bandwidth:") + String(bwOptions[b.build.value]));
                currentLoraBandwidth = bwOptionsValues[b.build.value];
                break;
            case DB_NAMESPACE::lora_coding_rate_selected:
                logger.println(String("current_lora_coding_rate:") + String(crOptions[b.build.value]));
                currentLoraCodingRate = String(crOptions[b.build.value]).toInt();
                break;
            case DB_NAMESPACE::lora_max_attempts:
                //logger.println(String("lora_max_attempts:") + String(b.build.value));
                currentLoraMaxAttempts = b.build.value.toInt();
                break;
            case DB_NAMESPACE::lora_tx_power:
                //logger.println(String("lora_tx_power:") + String(b.build.value));
                currentLoraTxPower = b.build.value.toInt();
                break;
        }

        if (b.Button(H("apply_lora"), "Применить настройки LoRa")) {
            _db->update(DB_NAMESPACE::lora_spreading, currentLoraSpreading);
            _db->update(DB_NAMESPACE::lora_bandwidth, currentLoraBandwidth.toFloat());
            _db->update(DB_NAMESPACE::lora_coding_rate, currentLoraCodingRate);
            _db->update(DB_NAMESPACE::lora_max_attempts, currentLoraMaxAttempts);
            _db->update(DB_NAMESPACE::lora_tx_power, currentLoraTxPower);
            loraManager->applySettings();
            _needRestart = true; // Отметка о необходимости перезагрузки
        }
    }

    // Управление устройством
    {
        //sets::Group g(b, "Управление устройством");
        if (b.Button(DB_NAMESPACE::restart_device, "Перезагрузить устройство")) {
            logger.println("Запрос на перезагрузку устройства...");
            _needRestart = true;
        }
    }

}

// Добавить реализацию метода buildSystemMonitorTab:
void UIBuilder::buildSystemMonitorTab(sets::Builder& b) {
    if (systemMonitor == nullptr) {
        b.Label("System monitoring not available");
        return;
    }

    systemMonitor->update(); // Обновляем данные мониторинга

    {
        sets::Group g(b, "CPU & Memory");
        b.Label("CPU Usage: " + String(systemMonitor->getTotalCpuUsage()) + "%");
        b.Label("Free Heap: " + String(systemMonitor->getFreeHeap() / 1024) + " kB");
        b.Label("Min Free Heap: " + String(systemMonitor->getMinFreeHeap() / 1024) + " kB");
        // UBaseType_t unusedStackWords = uxTaskGetStackHighWaterMark(NULL);
        // size_t unusedStackBytes = unusedStackWords * sizeof(StackType_t); // Обычно 4 байта
        // b.Label("Min Free Stack: " + String(unusedStackBytes / 1024) + " kB");
    }

    Serial.println("Отображение списка задач...");
    {
        sets::Group g(b, "Task Statistics");

        // Получаем информацию о задачах
        uint16_t taskCount = 0;
        SystemMonitor::TaskInfo* tasks = systemMonitor->getTasksInfo(taskCount); // Исправлено: SystemMonitor::TaskInfo* -> TaskInfo*

        if (tasks != nullptr && taskCount > 0) {
            for (uint16_t i = 0; i < taskCount; i++) {
                // Определяем текстовое состояние
                String state;
                switch (tasks[i].state) {
                    case eRunning:    state = "Running"; break;
                    case eReady:      state = "Ready"; break;
                    case eBlocked:    state = "Blocked"; break;
                    case eSuspended:  state = "Suspended"; break;
                    case eDeleted:    state = "Deleted"; break;
                    default:          state = "Unknown"; break;
                }

                b.Label(String(tasks[i].name) + ": "+ String(tasks[i].cpuUsage) + "[" + state + "] " + String(tasks[i].stackHighWater) + " bytes free");
            }
            // Внимание: Память для 'tasks' выделяется статически в getTasksInfo,
            // поэтому освобождать ее здесь не нужно.
            // vPortFree(tasks);
        } 
    }

    // Кнопка обновления
    if (b.Button(H("refreshStats"), "Refresh Statistics")) {
        systemMonitor->update();
        b.reload();
    }
}

// Проверка необходимости перезагрузки
bool UIBuilder::needsRestart() const {
    return _needRestart;
}