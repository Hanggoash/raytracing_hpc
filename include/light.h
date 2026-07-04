#ifndef RAYTRACING_HPC_LIGHT_H
#define RAYTRACING_HPC_LIGHT_H

#include "vec3.h"

struct Light {
    Vec3 position;
    Vec3 color;
    double intensity;

    Light() : position(), color(1.0, 1.0, 1.0), intensity(1.0) {}

    Light(const Vec3& lightPosition, const Vec3& lightColor, double lightIntensity)
        : position(lightPosition), color(lightColor), intensity(lightIntensity) {}
};

#endif
