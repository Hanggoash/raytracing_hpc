#ifndef RAYTRACING_HPC_VEC3_H
#define RAYTRACING_HPC_VEC3_H

#include <algorithm>
#include <cmath>
#include <ostream>

class Vec3 {
public:
    double x;
    double y;
    double z;

    Vec3() : x(0.0), y(0.0), z(0.0) {}
    Vec3(double value) : x(value), y(value), z(value) {}
    Vec3(double xValue, double yValue, double zValue) : x(xValue), y(yValue), z(zValue) {}

    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    Vec3 operator+(const Vec3& rhs) const {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vec3 operator-(const Vec3& rhs) const {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator/(double scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    Vec3& operator+=(const Vec3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vec3& operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3& operator/=(double scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    double lengthSquared() const {
        return x * x + y * y + z * z;
    }

    double length() const {
        return std::sqrt(lengthSquared());
    }

    Vec3 normalized() const {
        double len = length();
        if (len <= 1e-12) {
            return Vec3(0.0, 0.0, 0.0);
        }
        return *this / len;
    }

    Vec3 clamped(double low = 0.0, double high = 1.0) const {
        return Vec3(
            std::clamp(x, low, high),
            std::clamp(y, low, high),
            std::clamp(z, low, high)
        );
    }

    static int toByte(double value) {
        double corrected = std::pow(std::clamp(value, 0.0, 1.0), 1.0 / 2.2);
        return static_cast<int>(corrected * 255.0 + 0.5);
    }
};

inline Vec3 operator*(double scalar, const Vec3& v) {
    return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

inline double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline Vec3 multiply(const Vec3& a, const Vec3& b) {
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline Vec3 reflect(const Vec3& incident, const Vec3& normal) {
    return incident - 2.0 * dot(incident, normal) * normal;
}

inline Vec3 lerp(const Vec3& a, const Vec3& b, double t) {
    return a * (1.0 - t) + b * t;
}

inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

#endif
