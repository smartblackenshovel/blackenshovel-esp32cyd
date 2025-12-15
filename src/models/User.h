#pragma once
#include <Arduino.h>
#include "Location.h"

class User {
    private:
        String name;
        String id;
        Location loc;
    public:
        User(const String& userName, const String& userId)
            : name(userName), id(userId), loc(0, 0) {}
        String getName() const { return name; }
        String getId() const { return id; }
        void setLocation(double latitude, double longitude) {
            loc = Location(latitude, longitude);
        }
};
