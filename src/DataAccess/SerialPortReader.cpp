#include "SerialPortReader.h"
#include "JsonUtils.h"

void SerialPortReader::read() {
    if (!serialPort.available()) {
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, serialPort);
    String msgType = doc["msgType"].as<String>();
    if (msgType == "users") {
        sessionUpdater.setUsers(parseUsers(doc));
        return;
    }
}

std::vector<User> parseUsers(JsonDocument& doc) {
    std::vector<User> users;
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject obj : arr) {
        String name = obj["name"].as<String>();
        String id = obj["id"].as<String>();
        users.push_back(User(name, id));
    }
    return users;
}