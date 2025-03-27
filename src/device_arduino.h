#ifndef DEVICE_ARDUINO_H
#define DEVICE_ARDUINO_H

#include <WiFi101.h>  
#include <WifiClient.h>
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

    StaticJsonDocument<512> doc;  // Reduced size for memory efficiency
    char output[256];  
    WiFiClient client;
    bool isConnected = false;

    // Vector to store pending commands
    std::vector<Command> pendingCommands;

public:
    // Constructor
    Device(char* devId);

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

#endif // DEVICE_ARDUINO_H