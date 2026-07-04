#ifndef RAYTRACING_HPC_SPHERE_H
#define RAYTRACING_HPC_SPHERE_H

#include "object.h"

class Sphere : public Object {
public:
    Vec3 center;
    double radius;
    Material material;

    Sphere(const Vec3& sphereCenter, double sphereRadius, const Material& sphereMaterial)
        : center(sphereCenter), radius(sphereRadius), material(sphereMaterial) {}

    bool intersect(const Ray& ray, double tMin, double tMax, HitRecord& record) const override {
        Vec3 oc = ray.origin - center;
        double a = dot(ray.direction, ray.direction);
        double halfB = dot(oc, ray.direction);
        double c = dot(oc, oc) - radius * radius;
        double discriminant = halfB * halfB - a * c;

        if (discriminant < 0.0) {
            return false;
        }

        double sqrtD = std::sqrt(discriminant);
        double root = (-halfB - sqrtD) / a;
        if (root < tMin || root > tMax) {
            root = (-halfB + sqrtD) / a;
            if (root < tMin || root > tMax) {
                return false;
            }
        }

        record.hit = true;
        record.t = root;
        record.position = ray.at(root);
        Vec3 outwardNormal = (record.position - center) / radius;
        record.normal = dot(ray.direction, outwardNormal) < 0.0 ? outwardNormal : -outwardNormal;
        record.material = material;
        return true;
    }
};

#endif
