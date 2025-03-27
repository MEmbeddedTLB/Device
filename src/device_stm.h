#ifndef DEVICE_H
#define DEVICE_H

#include "stm32_hal.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <vector>

struct Command {
    String name;
    int status;
};

class Device {
private:
    // Server details
    String server_url;
    String deviceId;

    // API endpoints
    String connect_endpoint;
    String commands_endpoint;
    String data_endpoint;

    StaticJsonDocument<512> doc;  // JSON document for parsing
    char output[256];  // Buffer for serialized JSON
    WiFiClient client;
    bool isConnected = false;

    // Vector to store pending commands
    std::vector<Command> pendingCommands;

    // Private helper method for sending HTTP requests
    bool sendHttpRequest(const String& method, const String& endpoint, const String& payload = "");

public:
    // Constructor
    Device(const char* devId);

    // Destructor
    ~Device();

    // Initialization methods
    void initialize();
    void uninitialize();

    // Data sending methods
    bool sendData(String type, String name, String component, int status);
    bool sendData(String type, String name, String component, String status);
    bool sendData(String type, String name, String component, int status, JsonArray dataArray);

    // Connection and command methods
    bool connectWiFi(const char* ssid, const char* password);
    bool sendDeviceConnect();
    void checkForCommands();
    bool getConnectionStatus() const;

    // Method to access pending commands
    std::vector<Command>& getPendingCommands();
};

#endif // DEVICE_H