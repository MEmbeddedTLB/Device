#ifndef DEVICE_ESP_H
#define DEVICE_ESP_H

#include <WiFi.h>
#include <HTTPClient.h>
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

    DynamicJsonDocument doc;
    String output;
    HTTPClient httpClient;
    bool isConnected = false;

    // Vector to store pending commands
    std::vector<Command> pendingCommands;

public:
    // Constructor
    Device(char* devId);

    // Destructor to free dynamically allocated memory
    ~Device();

    // Initialization methods
    void initialize();
    void uninitialize();

    // Data sending methods
    void sendData(String type, String name, String component, int status);
    void sendData(String type, String name, String component, String status);
    void sendData(String type, String name, String component, int status, JsonArray dataArray);

    // Connection and command methods
    bool connectWiFi(const char* ssid, const char* password);
    bool sendDeviceConnect();
    void checkForCommands();
    bool getConnectionStatus() const;

    // Method to access pending commands
    std::vector<Command>& getPendingCommands();

    
};

#endif // DEVICE_ESP_H