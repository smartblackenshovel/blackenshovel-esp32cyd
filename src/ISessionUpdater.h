#pragma once
#include "models/User.h"
#include "models/IMU.h"
#include <vector>

class ISessionUpdater {
    public:
        virtual ~ISessionUpdater() {}
        virtual void setUsers(std::vector<User> users) = 0;
        virtual void setUserImu(double ax, double ay, double az, double gx, double gy, double gz) = 0;
};