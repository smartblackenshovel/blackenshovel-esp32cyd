#pragma once
#include <Arduino.h>
#include "../User.h"

class SerialPortWriter {
    public:
        SerialPortWriter(HardwareSerial& serial) : serialPort(serial) {}
        void writeUser(const User& user);
    private:
        HardwareSerial& serialPort;
};