#ifndef RAYTRACING_HPC_PLANE_H
#define RAYTRACING_HPC_PLANE_H

#include <cmath>

#include "object.h"

class Plane : public Object {
public:
    Vec3 point;
    Vec3 normal;
    Material material;

    Plane(const Vec3& planePoint, const Vec3& planeNormal, const Material& planeMaterial)
        : point(planePoint), normal(planeNormal.normalized()), material(planeMaterial) {}

    bool intersect(const Ray& ray, double tMin, double tMax, HitRecord& record) const override {
        double denom = dot(normal, ray.direction);
        if (std::fabs(denom) < 1e-8) {
            return false;
        }

        double t = dot(point - ray.origin, normal) / denom;
        if (t < tMin || t > tMax) {
            return false;
        }

        record.hit = true;
        record.t = t;
        record.position = ray.at(t);
        record.normal = denom < 0.0 ? normal : -normal;
        record.material = material;
        return true;
    }
};

#endif
