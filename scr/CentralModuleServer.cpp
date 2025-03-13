#include "CentralModuleServer.h"

CentralModuleServer::CentralModuleServer(int port) 
    : server(port), lastI2CRequestTime(0)яяяяя {
}

void CentralModuleServer::begin() {
    Serial.begin(115200);
    Wire.begin();  // Ініціалізація I2C на майстрі
    // Ініціалізація TFT дисплея
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    // Ініціалізація файлової системи
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        tft.println("LittleFS Error!");
        return;
    } else {
        Serial.println("LittleFS mounted successfully");
    }

    // Створення точки доступу
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("Access Point started");
    IPAddress ip = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // Налаштування піну LED як вихід
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); // Вимкнення світлодіода на початку

    // Виведення IP-адреси на дисплей
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 30);
    tft.println("WiFi AP Started");
    tft.setCursor(10, 60);
    tft.println("SSID: " + String(ap_ssid));
    tft.setCursor(10, 90);
    tft.println("IP:");
    tft.setCursor(10, 120);
    tft.println(ip.toString());

    // Налаштування вебсерверу
    server.on("/", [this]() { handleRoot(); });
    server.on("/style.css", [this]() { handleCSS(); });
    server.on("/script.js", [this]() { handleJS(); });
    server.on("/setwifi", [this]() { handleSetWiFi(); });
    server.on("/connection-status", [this]() { handleConnectionStatus(); });
    server.on("/get-new-ip", [this]() { handleGetNewIP(); });
    server.on("/get-all-data", [this]() { handleGetAllData(); });
    server.on("/toggle-door-password", HTTP_POST, [this]() { handleToggleDoorWithPassword(); });
    server.on("/test-alarm", HTTP_POST, [this]() { handleTestAlarm(); });
    server.on("/start-stream", HTTP_GET, [this]() { startStreaming(); });
    server.on("/stop-stream", HTTP_GET, [this]() { stopStreaming(); });
    server.on("/toggle-led", HTTP_GET, [this]() { toggleLED(); });
    server.on("/capture-image", HTTP_GET, [this]() { captureImage(); });
    server.on("/photo.jpg", HTTP_GET, [this]() { serveImage(); });
    server.on("/update-settings", HTTP_POST, [this]() { updateCameraSettings(); });
    server.on("/update-connection", HTTP_POST, [this]() { updateConnection(); });
    server.on("/connection-status-sub2", HTTP_GET, [this]() { getConnectionStatus(); });
    server.on("/update-volume", HTTP_POST, [this]() { updateVolume(); });
    server.on("/change-password", HTTP_POST, [this]() { changePassword(); });

    server.begin();
    Serial.println("HTTP server started");
}

void CentralModuleServer::handleClient() {
    server.handleClient();
}

void CentralModuleServer::handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void CentralModuleServer::handleCSS() {
  File file = LittleFS.open("/style.css", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/css");
  file.close();
}

void CentralModuleServer::handleJS() {
  File file = LittleFS.open("/script.js", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "application/javascript");
  file.close();
}

void CentralModuleServer::handleSetWiFi() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String newSSID = server.arg("ssid");
    String newPassword = server.arg("password");

    server.send(200, "text/html", "<h1>Wi-Fi credentials saved. Reconnecting...</h1>");

    WiFi.softAPdisconnect(true);
    WiFi.begin(newSSID.c_str(), newPassword.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 10000) {
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to new Wi-Fi");
      Serial.println(WiFi.localIP());
      
      // Оновлення IP-адреси на дисплеї
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(10, 30);
      tft.println("Connected to Wi-Fi");
      tft.setCursor(10, 60);
      tft.println("IP:");
      tft.setCursor(10, 90);
      tft.println(WiFi.localIP().toString());
      
      // Конвертуємо String у char[] і заповнюємо структуру
      newSSID.toCharArray(wifiData.ssid, sizeof(wifiData.ssid));
      newPassword.toCharArray(wifiData.password, sizeof(wifiData.password));
      WiFi.localIP().toString().toCharArray(wifiData.central_module_ip, sizeof(wifiData.central_module_ip));
      // Відправка WiFi-даних
      Wire.beginTransmission(I2C_ADDRESS);
      Wire.write((uint8_t*)&wifiData, sizeof(wifiData));
      Wire.endTransmission();
      Serial.println("WiFi credentials sent to slave.");
    } else {
      Serial.println("Failed to connect to Wi-Fi");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(10, 30);
      tft.println("Failed to connect");
    }
  }
}

void CentralModuleServer::handleConnectionStatus() {
  String status = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";
  server.send(200, "application/json", "{\"status\":\"" + status + "\"}");
}

void CentralModuleServer::handleGetNewIP() {
  String ip = WiFi.localIP().toString();
  String jsonResponse = "{\"ip\":\"" + ip + "\"}";
  server.send(200, "application/json", jsonResponse);
}

void CentralModuleServer::handleGetAllData() {
    String jsonResponse = "{";
jsonResponse += "\"fire\":{\"temp\":" + String(fireData.temp) + 
                ",\"pres\":" + String(fireData.pres) + 
                ",\"hum\":" + String(fireData.hum) + 
                ",\"mq2\":" + String(fireData.mq2) + "},";
jsonResponse += "\"motion\":{\"motionDetected\":" + String(motionData.motionDetected ? "true" : "false") + 
                ",\"timeDetected\":\"" + String(motionData.timeDetected) + "\"},";
jsonResponse += "\"access\":{\"lastAttemptSuccess\":" + String(accessData.lastAttemptSuccess ? "true" : "false") + 
                ",\"totalAttempts\":" + String(accessData.totalAttempts) + "},";
jsonResponse += "\"alert\":{\"activeAlert\":" + String(alertData.activeAlert ? "true" : "false") + "},";
jsonResponse += "\"status\":{";
jsonResponse += "\"subsystem3\":" + String(status.subsystem3Connected ? "true" : "false") + ",";
jsonResponse += "\"subsystem4\":" + String(status.subsystem4Connected ? "true" : "false") + ",";
jsonResponse += "\"subsystem5\":" + String(status.subsystem5Connected ? "true" : "false") + ",";
jsonResponse += "\"subsystem6\":" + String(status.subsystem6Connected ? "true" : "false");
jsonResponse += "}}";

  server.send(200, "application/json", jsonResponse);
}

void CentralModuleServer::handleToggleDoorWithPassword() {
    if (access2.activeAccess) {
        // Якщо двері відкриті, виконуємо закриття без пароля
        closeDoor();
        server.send(200, "text/plain", "Двері закрито!");
    }
    else {
        // Якщо двері закриті, перевіряємо пароль
        if (server.hasArg("password")) {
            String inputPassword = server.arg("password");
            String storedPassword = readPassword();

            if (inputPassword == storedPassword) {
                openDoor();
                server.send(200, "text/plain", "Двері відкрито!");
            }
            else {
                server.send(403, "text/plain", "Невправильний пароль!");
            }
        }
        else {
            // Пароль не надано
            server.send(400, "text/plain", "Пароль не вказано!");
        }
    }
}

void CentralModuleServer::handleTestAlarm() {
    Serial.println("Активація перевірки сигналізації...");
    alert2.testAlert = true;
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((uint8_t*)&alert2, sizeof(alert2));
    Wire.endTransmission();
    server.send(200, "text/plain", "Сигналізацію активовано на кілька секунд!");
}

void CentralModuleServer::startStreaming() {
    WiFiClient client;
    if (client.connect(cam_ip, 80)) {
        client.print("GET /start-stream HTTP/1.1\r\n");
        client.print("Host: ");
        client.print(cam_ip);
        client.print("\r\nConnection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (!client.available()) {
            if (millis() - timeout > 5000) {
                Serial.println("ESP32-CAM не відповідає");
                server.send(504, "text/plain", "Помилка: ESP32-CAM не відповідає");
                return;
            }
        }
        Serial.println("Стрімінг розпочато");
        server.send(200, "text/plain", "Streaming started");
    }
    else {
        Serial.println("Не вдалося підключитися до ESP32-CAM");
        server.send(503, "text/plain", "Помилка: Не вдалося підключитися до ESP32-CAM");
    }
    client.stop();
}

void CentralModuleServer::stopStreaming() {
    WiFiClient client;
    if (client.connect(cam_ip, 80)) {
        client.print("GET /stop-stream HTTP/1.1\r\n");
        client.print("Host: ");
        client.print(cam_ip);
        client.print("\r\nConnection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (!client.available()) {
            if (millis() - timeout > 5000) {
                Serial.println("ESP32-CAM не відповідає");
                server.send(504, "text/plain", "Помилка: ESP32-CAM не відповідає");
                return;
            }
        }
        Serial.println("Стрімінг зупинено");
        server.send(200, "text/plain", "Streaming stopped");
    }
    else {
        Serial.println("Не вдалося підключитися до ESP32-CAM");
        server.send(503, "text/plain", "Помилка: Не вдалося підключитися до ESP32-CAM");
    }
    client.stop();
}

void CentralModuleServer::toggleLED() {
    // Надсилання запиту на ESP32-CAM для увімкнення/вимкнення LED
    WiFiClient client;
    if (client.connect(cam_ip, 80)) {
        client.println("GET /toggle-led HTTP/1.1");
        client.println("Host: " + String(cam_ip));
        client.println("Connection: close");
        client.println();
        Serial.println("Запит до ESP32-CAM на зміну стану LED надіслано");
    }
    else {
        Serial.println("Не вдалося підключитися до ESP32-CAM");
    }
    server.send(200, "text/plain", "Запит до ESP32-CAM на зміну стану LED виконано");
}

void CentralModuleServer::captureImage() {
    // Надсилання запиту на ESP32-CAM для захоплення фото
    WiFiClient client;
    if (client.connect(cam_ip, 80)) {
        client.println("GET /capture HTTP/1.1");
        client.println("Host: " + String(cam_ip));
        client.println("Connection: close");
        client.println();
        Serial.println("Запит до ESP32-CAM на захоплення фото надіслано");

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                break;
            }
        }

        File file = LittleFS.open("/photo.jpg", "w");
        if (!file) {
            Serial.println("Не вдалося відкрити файл для запису");
            return;
        }

        while (client.connected()) {
            int len = client.available();
            if (len > 0) {
                uint8_t buf[len];
                client.readBytes(buf, len);
                file.write(buf, len);
            }
        }

        file.close();
        Serial.println("Фото збережено");
        server.send(200, "text/plain", "Фото збережено");
    }
    else {
        Serial.println("Не вдалося підключитися до ESP32-CAM");
        server.send(500, "text/plain", "Не вдалося підключитися до ESP32-CAM");
    }
}

void CentralModuleServer::serveImage() {
    File file = LittleFS.open("/photo.jpg", "r");
    if (!file) {
        server.send(404, "text/plain", "Фото не знайдено");
        return;
    }
    server.streamFile(file, "image/jpeg");
    file.close();
}

void CentralModuleServer::updateCameraSettings() {
    if (server.hasArg("plain") == false) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    String body = server.arg("plain");
    WiFiClient client;
    if (client.connect(cam_ip, 80)) {
        client.println("POST /update-settings HTTP/1.1");
        client.println("Host: " + String(cam_ip));
        client.println("Content-Type: application/json");
        client.println("Content-Length: " + String(body.length()));
        client.println("Connection: close");
        client.println();
        client.println(body);

        unsigned long timeout = millis();
        while (!client.available()) {
            if (millis() - timeout > 5000) {
                Serial.println("ESP32-CAM не відповідає");
                server.send(504, "text/plain", "Помилка: ESP32-CAM не відповідає");
                return;
            }
        }

        String line;
        while (client.available()) {
            line = client.readStringUntil('\n');
        }
        Serial.println("Налаштування камери оновлено");
        server.send(200, "text/plain", "Settings Updated");
    }
    else {
        Serial.println("Не вдалося підключитися до ESP32-CAM");
        server.send(503, "text/plain", "Помилка: Не вдалося підключитися до ESP32-CAM");
    }
    client.stop();
}

void CentralModuleServer::updateConnection() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    String body = server.arg("plain");
    Serial.println("Отримано запит: " + body);

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        Serial.println("Помилка десеріалізації: " + String(error.c_str()));
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    subsystem2Connected = doc["subsystem2Connected"];
    const char* cam_ip_received = doc["cam_ip"];
    strncpy(cam_ip, cam_ip_received, sizeof(cam_ip) - 1);

    Serial.println("Статус підключення оновлено:");
    Serial.println("subsystem2Connected: " + String(subsystem2Connected));
    Serial.println("cam_ip: " + String(cam_ip));

    server.send(200, "text/plain", "Connection status updated");
}

void CentralModuleServer::getConnectionStatus() {
    Serial.println("Запит на /connection-status-sub2");

    // Формуємо текстову відповідь
    String response = String(subsystem2Connected ? "Підключено" : "Немає зв'язку") +
        "|" + String(cam_ip);

    // Відправляємо відповідь
    server.send(200, "text/plain", response);
}

void CentralModuleServer::updateVolume() {
    if (server.hasArg("plain") == false) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    int volume = doc["volume"];
    if (volume < 0 || volume > 30) {
        server.send(400, "text/plain", "Volume out of range");
        return;
    }

    // Пересилання гучності на модуль сигналізації по I2C
    alert3.volume = volume;
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((uint8_t*)&alert3, sizeof(alert3));
    Wire.endTransmission();
}

void CentralModuleServer::changePassword() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Помилка: Дані не отримано");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "text/plain", "Помилка: Некоректний JSON");
        return;
    }

    const char* oldPassword = doc["oldPassword"];
    const char* newPassword = doc["newPassword"];

    if (strlen(newPassword) < 6) {
        server.send(400, "text/plain", "Помилка: Пароль повинен містити щонайменше 6 символів");
        return;
    }

    // Ініціалізація LittleFS
    if (!LittleFS.begin()) {
        server.send(500, "text/plain", "Помилка: Не вдалося ініціалізувати LittleFS");
        return;
    }

    // Читання збереженого пароля
    File passwordFile = LittleFS.open("/password.txt", "r");
    if (!passwordFile) {
        server.send(500, "text/plain", "Помилка: Не вдалося відкрити файл для читання");
        return;
    }

    String storedPassword = passwordFile.readStringUntil('\n');
    storedPassword.trim(); // Видаляємо зайві символи
    passwordFile.close();

    // Перевірка старого пароля
    if (String(oldPassword) != storedPassword) {
        server.send(403, "text/plain", "Помилка: Невірний старий пароль");
        return;
    }

    // Запис нового пароля
    passwordFile = LittleFS.open("/password.txt", "w");
    if (!passwordFile) {
        server.send(500, "text/plain", "Помилка: Не вдалося відкрити файл для запису");
        return;
    }

    passwordFile.print(newPassword);
    passwordFile.close();

    Serial.println("Новий пароль збережено: " + String(newPassword));

    server.send(200, "text/plain", "Пароль успішно змінено");
}

void CentralModuleServer::requestDataFromSlave() {
    if (millis() - lastI2CRequestTime >= 1000) {  // Опитування I2C кожну секунду
        lastI2CRequestTime = millis();

        // Запит даних від підлеглого
        if (Wire.requestFrom(I2C_ADDRESS, sizeof(fireData) + sizeof(motionData) + sizeof(accessData) + sizeof(alertData)) == (sizeof(fireData) + sizeof(motionData) + sizeof(accessData) + sizeof(alertData))) {
            Wire.readBytes((char*)&fireData, sizeof(fireData));
            status.subsystem5Connected = fireData.subsystem5Connected;

            Wire.readBytes((char*)&motionData, sizeof(motionData));
            status.subsystem4Connected = motionData.subsystem4Connected;

            Wire.readBytes((char*)&accessData, sizeof(accessData));
            status.subsystem3Connected = accessData.subsystem3Connected;

            Wire.readBytes((char*)&alertData, sizeof(alertData));
            status.subsystem6Connected = alertData.subsystem6Connected;
            digitalWrite(ledPin, HIGH);
        }
        else {
            // Обробка помилок
            Serial.println("Помилка отримання даних від підлеглого!");
            digitalWrite(ledPin, LOW);
        }
    }
}

String CentralModuleServer::readPassword() {
    File file = LittleFS.open("/password.txt", "r");
    if (!file) {
        Serial.println("Failed to open password file");
        return "";
    }
    String password = file.readStringUntil('\n');
    password.trim(); // Видаляємо зайві пробіли та символи нового рядка
    file.close();
    return password;
}

void CentralModuleServer::openDoor() {
    // Код для відкриття дверей
    Serial.println("Двері відкрито!");
    access2.activeAccess = true;
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((uint8_t*)&access2, sizeof(access2));
    Wire.endTransmission();
    Serial.println("Access data sent to slave.");
}

void CentralModuleServer::closeDoor() {
    // Код для закриття дверей
    Serial.println("Двері закрито!");
    access2.activeAccess = false;
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write((uint8_t*)&access2, sizeof(access2));
    Wire.endTransmission();
    Serial.println("Access data sent to slave.");
}