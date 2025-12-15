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

void SessionManager::setUserLoc(double lat, double lon) {
    selectedUser->setLocation(lat, lon);
}

void SessionManager::setSpot(String id, double lat, double lon) {
    if (!nextSpot->isComplete()) { return; }
    if (nextSpot != nullptr) { delete nextSpot; }
    nextSpot = new Spot(id, lat, lon);
}
