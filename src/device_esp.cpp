#include "device_esp.h"

// Constructor
Device::Device(char* devId) 
    : doc(1024) {
  deviceId = devId;
  server_url = "https://devica.membeddedtechlab.com";
  connect_endpoint = server_url+ "/api/device-connect";
  commands_endpoint = server_url + "/api/device-commands/" + deviceId;
  data_endpoint = server_url + "/api/device-data";
}

// Destructor to free dynamically allocated memory


void Device::initialize() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    httpClient.begin(data_endpoint);
    httpClient.addHeader("Content-Type", "application/json");
}

void Device::sendData(String type, String name, String component, int status) {
    doc.clear();
    doc["type"] = type;
    doc["deviceId"] = deviceId;
    doc["name"] = name;
    doc["component"] = component;
    doc["status"] = status;
    
    serializeJson(doc, output);
    int httpResponseCode = httpClient.POST(output);
    
    if (httpResponseCode <= 0) {
        Serial.printf("Error sending data for %s: %d\n", name, httpResponseCode);
    }
}

void Device::sendData(String type, String name, String component, int status, JsonArray dataArray) {
    doc.clear();
    doc["type"] = type;
    doc["deviceId"] = deviceId;
    doc["name"] = name;
    doc["component"] = component;
    doc["status"] = status;
    doc["data"] = dataArray;
    
    serializeJson(doc, output);
    int httpResponseCode = httpClient.POST(output);
    
    if (httpResponseCode <= 0) {
        Serial.printf("Error sending data for %s: %d\n", name, httpResponseCode);
    }
}

void Device::sendData(String type, String name, String component, String status) {
    doc.clear();
    doc["type"] = type;
    doc["deviceId"] = deviceId;
    doc["name"] = name;
    doc["component"] = component;
    doc["status"] = status;
    
    serializeJson(doc, output);
    int httpResponseCode = httpClient.POST(output);
    
    if (httpResponseCode <= 0) {
        Serial.printf("Error sending data for %s: %d\n", name, httpResponseCode);
    }
}

void Device::uninitialize() {
    httpClient.end();
}

bool Device::connectWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    // Set a timeout for WiFi connection attempts
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) { // 10 second timeout
        delay(500);
        Serial.print(".");
        timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        return sendDeviceConnect();
    } else {
        Serial.println("\nFailed to connect to WiFi. Will retry in next loop.");
        return false;
    }
}

bool Device::sendDeviceConnect() {
    if (WiFi.status() == WL_CONNECTED && deviceId != nullptr) {
        // Prepare the JSON payload
        StaticJsonDocument<128> doc; // Smaller document size is sufficient
        doc["type"] = "deviceConnect";
        doc["deviceId"] = deviceId;
        
        String output;
        serializeJson(doc, output);

        // Send HTTP POST request
        HTTPClient connectHttpClient;
        connectHttpClient.begin(connect_endpoint);
        connectHttpClient.addHeader("Content-Type", "application/json");
        int httpResponseCode = connectHttpClient.POST(output);
        
        if (httpResponseCode > 0) {
            Serial.print("Device connect HTTP Response code: ");
            Serial.println(httpResponseCode);
            isConnected = true;
        } else {
            Serial.print("Device connect error code: ");
            Serial.println(httpResponseCode);
            isConnected = false;
        }
        
        connectHttpClient.end();
        return isConnected;
    }
    return false;
}

void Device::checkForCommands() {
    if (WiFi.status() == WL_CONNECTED && deviceId != nullptr) {
    
        // Recreate the HTTPClient to ensure a fresh connection
        HTTPClient httpClient;
        httpClient.begin(commands_endpoint);
        // Serial.println(commands_endpoint);
        
        int httpResponseCode = httpClient.GET();
        
        // Serial.print("Command Check Response Code: ");
        // Serial.println(httpResponseCode);
        
        if (httpResponseCode > 0) {
            String payload = httpClient.getString();
            // Serial.println("Payload received: " + payload);
            
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                // Clear previous pending commands
                pendingCommands.clear();
                
                // Check if "commands" array exists in the JSON
                if (doc.containsKey("commands") && doc["commands"].is<JsonArray>()) {
                    JsonArray commands = doc["commands"].as<JsonArray>();
                    
                    for (JsonVariant commandObj : commands) {
                        // Create a new Command struct and add to pending commands
                        Command newCommand;
                        newCommand.name = commandObj["name"].as<String>();
                        newCommand.status = commandObj["status"].as<int>();
                        
                        pendingCommands.push_back(newCommand);
                        
                        // Debug print
                        Serial.println("Received command: " + newCommand.name + 
                                     ", Status: " + String(newCommand.status));
                    }
                } else {
                    Serial.println("No commands array found in response");
                }
            } else {
                Serial.print("JSON deserialization failed: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.print("HTTP Error code: ");
            Serial.println(httpResponseCode);
            
            // Print out more detailed error information
            String errorMessage = httpClient.errorToString(httpResponseCode);
            Serial.println("Error details: " + errorMessage);
        }
        
        httpClient.end();
    } else {
        Serial.println("WiFi not connected or deviceId is null during command check");
    }
}

bool Device::getConnectionStatus() const {
    return isConnected;
}

std::vector<Command>& Device::getPendingCommands() {
    return pendingCommands;
}