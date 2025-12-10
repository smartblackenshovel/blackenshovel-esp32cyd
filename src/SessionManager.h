#include <Arduino.h>
#include <vector>
#include "ISessionUpdater.h"
#include "User.h"

class SessionManager : public ISessionUpdater {

    public:
        SessionManager() {}
        void setUsers(std::vector<User> newUsers) override;
        std::vector<User> getUsers() const { return users; }
    private:
        std::vector<User> users;
};  
