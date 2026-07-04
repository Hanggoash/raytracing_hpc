#ifndef RAYTRACING_HPC_MATERIAL_H
#define RAYTRACING_HPC_MATERIAL_H

#include "vec3.h"

struct Material {
    Vec3 baseColor;
    double diffuse;
    double specular;
    double shininess;
    double reflection;

    Material()
        : baseColor(0.8, 0.8, 0.8),
          diffuse(0.85),
          specular(0.35),
          shininess(48.0),
          reflection(0.0) {}

    Material(const Vec3& color, double diffuseValue, double specularValue,
             double shininessValue, double reflectionValue)
        : baseColor(color),
          diffuse(diffuseValue),
          specular(specularValue),
          shininess(shininessValue),
          reflection(reflectionValue) {}
};

#endif
