#include "utils/JsonUtils.h"
#include "SerialPortReader.h"

std::vector<User> parseUsers(JsonDocument& doc) {
    std::vector<User> users;
    JsonArray arr = doc["data"].as<JsonArray>();
    for (JsonObject obj : arr) {
        String name = obj["name"].as<String>();
        String id = obj["id"].as<String>();
        users.push_back(User(name, id));
    }
    return users;
}

void SerialPortReader::read() {
    if (!serialPort.available()) {
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, serialPort);
    if (error) {
        Serial.print("Failed to parse JSON from Serial Port: ");
        Serial.println(error.c_str());
        return;
    }
    String msgType = doc["msgType"].as<String>();
    msgType.trim();
    if (msgType == "users") {
        sessionUpdater.setUsers(parseUsers(doc));
        return;
    } else if (msgType == "imu") {
        // TO DO
    } else if (msgType == "userLoc") {
        // TO DO
    } else if (msgType == "spotLoc") {
        // TO DO 
    }
}
