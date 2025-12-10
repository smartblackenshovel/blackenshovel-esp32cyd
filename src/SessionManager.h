#pragma once
#include <Arduino.h>
#include <vector>
#include "DataAccess/SerialPortWriter.h"
#include "ISessionUpdater.h"
#include "User.h"

class SessionManager : public ISessionUpdater {

    public:
        SessionManager(SerialPortWriter& serialPortWriter);
        void setUsers(std::vector<User> newUsers) override;
        std::vector<User> getUsers() const { return users; }
        void selectUser(String name);
        String name = "Lauro";
    private:
        std::vector<User> users;
        User* selectedUser = nullptr;
        SerialPortWriter& serialPortWriter;
};  
