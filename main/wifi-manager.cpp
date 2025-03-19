#include "wifi-manager.h"

// Глобальный экземпляр менеджера WiFi
WiFiManager* wifiManager = nullptr;

// Конструктор
WiFiManager::WiFiManager(GyverDB* db) : _db(db) {
}

// Применение настроек WiFi из базы данных
void WiFiManager::applySettings() {
    int mode = _db->get(DB_NAMESPACE::wifi_mode).toInt();
    String apSSID = _db->get(DB_NAMESPACE::ap_ssid);
    String apPass = _db->get(DB_NAMESPACE::ap_pass);
    String staSSID = _db->get(DB_NAMESPACE::sta_ssid);
    String staPass = _db->get(DB_NAMESPACE::sta_pass);
    
    logger.println("Применение настроек WiFi...");
    
    switch (mode) {
        case MY_WIFI_MODE_AP:
            WiFi.mode(WIFI_AP);
            break;
        case MY_WIFI_MODE_STA:
            WiFi.mode(WIFI_STA);
            break;
        case MY_WIFI_MODE_AP_STA:
            WiFi.mode(WIFI_AP_STA);
            break;
    }
    
    if (mode == MY_WIFI_MODE_AP || mode == MY_WIFI_MODE_AP_STA) {
        if (WiFi.softAP(apSSID.c_str(), apPass.length() > 0 ? apPass.c_str() : NULL)) {
            logger.println("Точка доступа: " + apSSID);
        }
    }
    
    if (mode == MY_WIFI_MODE_STA || mode == MY_WIFI_MODE_AP_STA) {
        WiFi.begin(staSSID.c_str(), staPass.c_str());
    }
    
    _db->update(DB_NAMESPACE::wifi_mode, mode);
    _db->update(DB_NAMESPACE::ap_ssid, apSSID);
    _db->update(DB_NAMESPACE::ap_pass, apPass);
    _db->update(DB_NAMESPACE::sta_ssid, staSSID);
    _db->update(DB_NAMESPACE::sta_pass, staPass);
}


// Получение статуса подключения
String WiFiManager::getStatusText() {
    String status;
    
    switch (WiFi.getMode()) {
        case WIFI_AP:
            status = "Access Point (AP) mode";
            break;
        case WIFI_STA:
            if (WiFi.status() == WL_CONNECTED) {
                status = "Connected to " + WiFi.SSID() + " (IP: " + WiFi.localIP().toString() + ")";
            } else {
                status = "Not connected to WiFi";
            }
            break;
        case WIFI_AP_STA:
            if (WiFi.status() == WL_CONNECTED) {
                status = "AP+STA: Connected to " + WiFi.SSID() + " (IP: " + WiFi.localIP().toString() + ")";
            } else {
                status = "AP+STA: Access Point active, no WiFi connection";
            }
            status += "\nAP: " + String(WiFi.softAPSSID()) + " (IP: " + WiFi.softAPIP().toString() + ")";
            break;
        default:
            status = "WiFi disabled";
    }
    
    return status;
}

// Инициализация значений WiFi по умолчанию
void WiFiManager::initDefaults() {
    _db->init(DB_NAMESPACE::wifi_mode, MY_WIFI_MODE_AP_STA);
    _db->init(DB_NAMESPACE::ap_ssid, DEFAULT_AP_SSID);
    _db->init(DB_NAMESPACE::ap_pass, DEFAULT_AP_PASS);
    _db->init(DB_NAMESPACE::sta_ssid, "");
    _db->init(DB_NAMESPACE::sta_pass, "");
}