#pragma once
#include "models/User.h"
#include <vector>

class ISessionUpdater {
    public:
        virtual ~ISessionUpdater() {}
        virtual void setUsers(std::vector<User> users) = 0;
};