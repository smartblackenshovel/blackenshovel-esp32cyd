#pragma once
class Location {
    private:
        double latitude;
        double longitude;
        double x;
        double y;
    public:
        Location(double lat, double lon, double x, double y) : latitude(lat), longitude(lon), x(x), y(y) {}
        double getLatitude() const { return latitude; }
        double getLongitude() const { return longitude; }
        double getX() const { return x; }
        double getY() const { return y; }
};