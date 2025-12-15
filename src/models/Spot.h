#pragma once
#include <Arduino.h>
#include "Location.h"

class Spot {
    private:
        String id;
        Location loc;
        bool isCompleted;
    public:
        Spot(const String& spotId, double lat, double lon)
            : id(spotId), loc(lat, lon), isCompleted(false) {}
        String getId() const { return id; }
        void setLocation(double latitude, double longitude) {
            loc = Location(latitude, longitude);
        }
        Location getLocation() { return loc; }
        void complete() { isCompleted = true; }
        bool isComplete() { return isCompleted; }
};
