#include "device_arduino.h"

// Constructor
Device::Device(char* devId) {
    deviceId = devId;
    server_url = "https://devica.membeddedtechlab.com";
    connect_endpoint = server_url + "/api/device-connect";
    commands_endpoint = server_url + "/api/device-commands/" + deviceId;
    data_endpoint = server_url + "/api/device-data";
}

// Destructor (can be empty for now)
Device::~Device() {}

void Device::initialize() {
    // Optional: Any initialization logic
}

bool Device::sendData(String type, String name, String component, int status) {
    if (WiFi.status() != WL_CONNECTED) return false;

    doc.clear();
    doc["type"] = type;
    doc["deviceId"] = deviceId;
    doc["name"] = name;
    doc["component"] = component;
    doc["status"] = status;

    // Serialize JSON to output buffer
    serializeJson(doc, output);

    // Connect and send data
    if (client.connect(server_url.c_str(), 8000)) {
        client.println("POST " + data_endpoint + " HTTP/1.1");
        client.println("Host: " + server_url);
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.print("Content-Length: ");
        client.println(measureJsonPretty(doc));
        client.println();
        client.println(output);

        // Wait for response
        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                client.stop();
                Serial.println("Client Timeout!");
                return false;
            }
        }

        client.stop();
        return true;
    }
    return false;
}

bool Device::sendData(String type, String name, String component, String status) {
    if (WiFi.status() != WL_CONNECTED) return false;

    doc.clear();
    doc["type"] = type;
    doc["deviceId"] = deviceId;
    doc["name"] = name;
    doc["component"] = component;
    doc["status"] = status;

    serializeJson(doc, output);

    // Similar connection and sending logic as previous method
    if (client.connect(server_url.c_str(), 8000)) {
        client.println("POST " + data_endpoint + " HTTP/1.1");
        client.println("Host: " + server_url);
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.print("Content-Length: ");
        client.println(measureJsonPretty(doc));
        client.println();
        client.println(output);

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                client.stop();
                Serial.println("Client Timeout!");
                return false;
            }
        }

        client.stop();
        return true;
    }
    return false;
}

bool Device::sendData(String type, String name, String component, int status, JsonArray dataArray) {
    if (WiFi.status() != WL_CONNECTED) return false;

    doc.clear();
    doc["type"] = type;
    doc["deviceId"] = deviceId;
    doc["name"] = name;
    doc["component"] = component;
    doc["status"] = status;
    doc["data"] = dataArray;

    serializeJson(doc, output);

    // Similar connection and sending logic as previous methods
    if (client.connect(server_url.c_str(), 8000)) {
        client.println("POST " + data_endpoint + " HTTP/1.1");
        client.println("Host: " + server_url);
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.print("Content-Length: ");
        client.println(measureJsonPretty(doc));
        client.println();
        client.println(output);

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                client.stop();
                Serial.println("Client Timeout!");
                return false;
            }
        }

        client.stop();
        return true;
    }
    return false;
}

bool Device::connectWiFi(const char* ssid, const char* password) {
    // Initialize WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    // Wait for connection with timeout
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return sendDeviceConnect();
    } else {
        Serial.println("\nFailed to connect to WiFi");
        return false;
    }
}

bool Device::sendDeviceConnect() {
    if (WiFi.status() == WL_CONNECTED && deviceId != nullptr) {
        doc.clear();
        doc["type"] = "deviceConnect";
        doc["deviceId"] = deviceId;

        serializeJson(doc, output);

        // Connect and send device connection
        if (client.connect(server_url.c_str(), 8000)) {
            client.println("POST " + connect_endpoint + " HTTP/1.1");
            client.println("Host: " + server_url);
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.print("Content-Length: ");
            client.println(measureJsonPretty(doc));
            client.println();
            client.println(output);

            // Wait for response
            unsigned long timeout = millis();
            while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                    client.stop();
                    Serial.println("Device Connect Timeout!");
                    isConnected = false;
                    return false;
                }
            }

            client.stop();
            isConnected = true;
            return true;
        }
    }
    return false;
}

void Device::checkForCommands() {
    if (WiFi.status() == WL_CONNECTED && deviceId != nullptr) {
        // Connect and send GET request
        if (client.connect(server_url.c_str(), 8000)) {
            client.println("GET " + commands_endpoint + " HTTP/1.1");
            client.println("Host: " + server_url);
            client.println("Connection: close");
            client.println();

            // Wait for response
            unsigned long timeout = millis();
            while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                    client.stop();
                    Serial.println("Command Check Timeout!");
                    return;
                }
            }

            // Skip HTTP headers
            while (client.available()) {
                String line = client.readStringUntil('\n');
                if (line == "\r") break;
            }

            // Read JSON payload
            String payload = client.readString();
            client.stop();

            // Parse JSON
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                pendingCommands.clear();

                if (doc.containsKey("commands") && doc["commands"].is<JsonArray>()) {
                    JsonArray commands = doc["commands"].as<JsonArray>();

                    for (JsonVariant commandObj : commands) {
                        Command newCommand;
                        newCommand.name = commandObj["name"].as<String>();
                        newCommand.status = commandObj["status"].as<int>();

                        pendingCommands.push_back(newCommand);
                        Serial.println("Received command: " + newCommand.name + 
                                     ", Status: " + String(newCommand.status));
                    }
                }
            } else {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
            }
        }
    }
}

void Device::uninitialize() {
    // Optional cleanup
    WiFi.disconnect();
}

bool Device::getConnectionStatus() const {
    return isConnected;
}

std::vector<Command>& Device::getPendingCommands() {
    return pendingCommands;
}