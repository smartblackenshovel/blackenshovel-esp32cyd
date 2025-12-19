#include "utils/JsonUtils.h"
#include "SerialPortWriter.h"

void SerialPortWriter::writeUser(const User& user) {
    JsonDocument doc;
    doc["msgType"] = "selectedUser";
    doc["data"]["id"] = user.getId();
    serializeJson(doc, serialPort);
};

void SerialPortWriter::writeFinishedSpot(const Spot& spot) {
    Serial.println("Writing that spot is finished");
    JsonDocument doc;
    doc["msgType"] = "completedSpot";
    doc["data"]["id"] = spot.getId();
    serializeJson(doc, serialPort);
}
