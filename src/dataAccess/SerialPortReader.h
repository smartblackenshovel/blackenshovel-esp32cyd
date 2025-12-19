#pragma once
#include <Arduino.h>
#include "ISessionUpdater.h"

class SerialPortReader {
    public:
        SerialPortReader(HardwareSerial& serial, ISessionUpdater& sessionUpdater) : serialPort(serial), sessionUpdater(sessionUpdater) {}
        void read();
    private:
        HardwareSerial& serialPort;
        ISessionUpdater& sessionUpdater;
};