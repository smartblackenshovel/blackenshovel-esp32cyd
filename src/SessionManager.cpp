#include "SessionManager.h"

SessionManager::SessionManager(SerialPortWriter& serialPortWriter)
    : serialPortWriter(serialPortWriter) {}

void SessionManager::setUsers(std::vector<User> newUsers) {
    if (!users.empty()) {return;}
    users = newUsers;
}

void SessionManager::selectUser(String name) {
    for (User& user : users) {
        if (user.getName() == name) {
            selectedUser = &user;
            serialPortWriter.writeUser(user);
        }
    }
}

void SessionManager::setUserImu(double ax, double ay, double az, double gx, double gy, double gz) {
    selectedUser->imu.setAccelerometer(ax, ay, az);
    selectedUser->imu.setGyroscope(gx, gy, gz);
}

void SessionManager::setUserLoc(double lat, double lon, double x, double y) {
    selectedUser->setLocation(lat, lon, x, y);
}

void SessionManager::setNextSpot(String id, double lat, double lon, double x, double y) {
    if (nextSpot != nullptr) {
        if (!nextSpot->isComplete()) {
            return;
        }
        delete nextSpot;
    }
    nextSpot = new Spot(id, lat, lon, x, y);
}

