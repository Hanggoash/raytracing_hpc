#ifndef RAYTRACING_HPC_RAY_H
#define RAYTRACING_HPC_RAY_H

#include "vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray() : origin(), direction(0.0, 0.0, -1.0) {}

    Ray(const Vec3& rayOrigin, const Vec3& rayDirection)
        : origin(rayOrigin), direction(rayDirection.normalized()) {}

    Vec3 at(double t) const {
        return origin + direction * t;
    }
};

#endif
