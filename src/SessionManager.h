#pragma once
#include <Arduino.h>
#include <vector>
#include "dataAccess/SerialPortWriter.h"
#include "ISessionUpdater.h"
#include "models/User.h"

class SessionManager : public ISessionUpdater {

    public:
        SessionManager(SerialPortWriter& serialPortWriter);
        void setUsers(std::vector<User> newUsers) override;
        std::vector<User> getUsers() const { return users; }
        void selectUser(String name);
        User* getSessionUser() const { return selectedUser; }
    private:
        std::vector<User> users;
        User* selectedUser = nullptr;
        SerialPortWriter& serialPortWriter;
};  
