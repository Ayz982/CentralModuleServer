# **CentralModuleServer**
**📌 A library for implementing a web server for an IoT-based comprehensive security system**

## **📖 Description**  
**CentralModuleServer** is a library for deploying a web server on a **LilyGo T-Display ESP32** that manages a **comprehensive security system**. It provides:  

✅ **Monitoring and management of subsystems** (video surveillance, motion sensors, fire safety, access control, etc.).  
✅ **Data exchange via I2C** to communicate with subordinate modules.  
✅ **Configuration storage** using LittleFS.  
✅ **Built-in access point** (AP Mode) for standalone management.  
✅ **Flexible API** for obtaining system status and configuring parameters.  

## **🚀 Installation**  
1️⃣ **Download via GitHub**  
Clone the repository into the Arduino libraries folder:  
```sh
cd ~/Documents/Arduino/libraries
git clone https://github.com/Ayz982/CentralModuleServer CentralModuleServer
```
2️⃣ **Add via ZIP**  
Download [the latest version](https://github.com/Ayz982/CentralModuleServer).  
Open **Arduino IDE → Sketch → Include Library →** Add .ZIP Library, and select the archive.

## **🛠 Usage**
```cpp
#include <CentralModuleServer.h>

CentralModuleServer server;

void setup() {
    Serial.begin(115200);
    server.begin(); // Start the web server
}

void loop() {
    server.handleClient(); // Handle HTTP requests
}
```
### To use this library, you need to include the following dependencies:  
✔️ **WiFi:** for connecting to Wi-Fi  
✔️ **LittleFS:** for the file system  
✔️ **TFT_eSPI:** for working with the TFT display   
✔️ **WebServer:** for setting up the web server  
✔️ **ArduinoJson:** for handling JSON  

## 🔗 **Main Features**  
🔹 **Wi-Fi access point:** standalone management via browser. 
🔹 **I2C communication:** data exchange with notification subsystems. 
🔹 **API implementation:** control and manage security parameters. 
🔹 **Status display** on the LilyGo T-Display.  

## 📋 **API**  
📌 **Retrieve subsystem status:**  
```bash
GET /status
```
📌 **Change Wi-Fi settings:**  
```bash
POST /setwifi {"ssid": "MyNetwork", "password": "MyPass"}
```
📌 **Request to open doors:**  
```bash
POST /opendoor
```
📌 **Activate alarm system:**  
```bash
POST /alarm {"state": true}
```

## 📜 **License**  
This library is distributed under the MIT license.  

## **✉ Feedback**  
If you have any questions or suggestions, feel free to contact [slyusarana@gmail.com](mailto:slyusarana@gmail.com).  
