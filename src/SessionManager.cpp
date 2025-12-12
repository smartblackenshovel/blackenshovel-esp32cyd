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
