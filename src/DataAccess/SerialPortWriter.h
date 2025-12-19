#pragma once
#include <Arduino.h>
#include "models/User.h"
#include "models/Spot.h"

class SerialPortWriter {
    public:
        SerialPortWriter(HardwareSerial& serial) : serialPort(serial) {}
        void writeUser(const User& user);
        void writeFinishedSpot(const Spot& spot);
    private:
        HardwareSerial& serialPort;
};