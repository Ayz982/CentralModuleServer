#ifndef CENTRAL_MODULE_SERVER_H
#define CENTRAL_MODULE_SERVER_H

#include <WiFi.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <WebServer.h>
#include <ArduinoJson.h>

class CentralModuleServer {
private:
    const char* ap_ssid = "ESP32_AP"; // SSID точки доступу
    const char* ap_password = "12345678"; // Пароль точки доступу
    const int ledPin = 12; // Пін для LED
    const int I2C_ADDRESS = 0x08; // Адреса I2C
    bool subsystem2Connected = false; // Статус підключення
    char cam_ip[16] = { 0 }; // IP-адреса камери

    struct Status {
        bool subsystem3Connected;
        bool subsystem4Connected;
        bool subsystem5Connected;
        bool subsystem6Connected;
    };

    struct MessageAlert {
        bool activeAlert;
        bool subsystem6Connected;
    };

    struct MessageFire {
        float temp;
        float pres;
        float hum;
        int mq2;
        bool subsystem5Connected;
    };

    struct MessageMotion {
        bool motionDetected;
        char timeDetected[20];
        bool subsystem4Connected;
    };

    struct MessageAccess {
        bool lastAttemptSuccess;
        int totalAttempts;
        bool subsystem3Connected;
    };

    struct MessageWiFi {
        char ssid[32];
        char password[32];
        char central_module_ip[16];
    };

    struct MessageAlert2 {
        bool testAlert;
        int numberTrack;
    };

    struct MessageAccess2 {
        bool activeAccess;
    };

    struct MessageAlert3 {
        int volume;
    };

    // Структури даних
    MessageAccess2 access2 = { false };
    MessageWiFi wifiData = { "MyWiFi", "MyPassword", "111.111.111.111" };
    MessageFire fireData;
    MessageMotion motionData;
    MessageAccess accessData;
    MessageAlert alertData;
    Status status = { false, false, false, false };
    MessageAlert2 alert2 = { false, 2 };
    MessageAlert3 alert3;

    WebServer server;
    TFT_eSPI tft;
    unsigned long lastI2CRequestTime;

    // Внутрішні функції обробки запитів
    void handleRoot();
    void handleCSS();
    void handleJS();
    void handleSetWiFi();
    void handleConnectionStatus();
    void handleGetNewIP();
    void handleGetAllData();
    void handleToggleDoorWithPassword();
    void handleTestAlarm();
    void startStreaming();
    void stopStreaming();
    void toggleLED();
    void captureImage();
    void serveImage();
    void updateCameraSettings();
    void updateConnection();
    void getConnectionStatus();
    void updateVolume();
    void changePassword();
    String readPassword(); 

public:
    CentralModuleServer(int port = 80);
    void begin();
    void handleClient();
    void requestDataFromSlave();
    void openDoor();
    void closeDoor();
};

#endif // CENTRAL_MODULE_SERVER_H