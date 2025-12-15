#pragma once
#include <Arduino.h>
#include <vector>
#include "dataAccess/SerialPortWriter.h"
#include "ISessionUpdater.h"
#include "models/User.h"
#include "models/Spot.h"
#include "models/IMU.h"

class SessionManager : public ISessionUpdater {

    public:
        SessionManager(SerialPortWriter& serialPortWriter);
        void setUsers(std::vector<User> newUsers) override;
        void setUserImu(double ax, double ay, double az, double gx, double gy, double gz) override;
        void setUserLoc(double lat, double lon) override;
        void setSpot(String id, double lat, double lon) override;
        std::vector<User> getUsers() const { return users; }
        void selectUser(String name);
        User* getSessionUser() const { return selectedUser; }
    private:
        std::vector<User> users;

        User* selectedUser = nullptr;
        Spot* nextSpot = nullptr;

        SerialPortWriter& serialPortWriter;
};  
