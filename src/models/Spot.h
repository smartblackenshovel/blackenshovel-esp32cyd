#pragma once
#include <Arduino.h>
#include "Location.h"

class Spot {
    private:
        String id;
        Location loc;
        bool isCompleted;
    public:
        Spot(const String& spotId, double lat, double lon, double x, double y)
            : id(spotId), loc(lat, lon, x, y), isCompleted(false) {}
        String getId() const { return id; }
        void setLocation(double latitude, double longitude, double x, double y) {
            loc = Location(latitude, longitude, x, y);
        }
        Location getLocation() { return loc; }
        void complete() { isCompleted = true; }
        bool isComplete() { return isCompleted; }
};
