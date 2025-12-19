#pragma once

class Accelerometer {
    private:
        double x, y, z;
    public:
        Accelerometer(double ax = 0, double ay = 0, double az = 0) : x(ax), y(ay), z(az) {}
        double getX() const { return x; }
        double getY() const { return y; }
        double getZ() const { return z; }
        void setValues(double ax, double ay, double az) { x = ax; y = ay; z = az; }
};

class Gyroscope {
    private:
        double x, y, z;
    public:
        Gyroscope(double gx = 0, double gy = 0, double gz = 0) : x(gx), y(gy), z(gz) {}
        double getX() const { return x; }
        double getY() const { return y; }
        double getZ() const { return z; }
        void setValues(double gx, double gy, double gz) { x = gx; y = gy; z = gz; }
};

class IMU {
    private:
        Accelerometer accel;
        Gyroscope gyro;

    public:
        IMU() : accel(), gyro() {}
        IMU(const Accelerometer& a, const Gyroscope& g) : accel(a), gyro(g) {}

        void setAccelerometer(double ax, double ay, double az) { accel.setValues(ax, ay, az); }
        void setGyroscope(double gx, double gy, double gz) { gyro.setValues(gx, gy, gz); }

        Accelerometer getAccelerometer() const { return accel; }
        Gyroscope getGyroscope() const { return gyro; }
};
