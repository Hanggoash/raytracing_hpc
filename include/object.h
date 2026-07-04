#ifndef RAYTRACING_HPC_OBJECT_H
#define RAYTRACING_HPC_OBJECT_H

#include "material.h"
#include "ray.h"

struct HitRecord {
    bool hit;
    double t;
    Vec3 position;
    Vec3 normal;
    Material material;

    HitRecord() : hit(false), t(0.0), position(), normal(), material() {}
};

class Object {
public:
    virtual ~Object() = default;
    virtual bool intersect(const Ray& ray, double tMin, double tMax, HitRecord& record) const = 0;
};

#endif
