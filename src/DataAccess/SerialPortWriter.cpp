#include "../JsonUtils.h"
#include "SerialPortWriter.h"

void SerialPortWriter::writeUser(const User& user) {
    JsonDocument doc;
    doc["msgType"] = "selectedUser";
    doc["data"]["id"] = user.getId();
    serializeJson(doc, serialPort);
};
