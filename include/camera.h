#ifndef RAYTRACING_HPC_CAMERA_H
#define RAYTRACING_HPC_CAMERA_H

#include <cmath>

#include "ray.h"

class Camera {
public:
    Vec3 position;
    Vec3 lookAt;
    Vec3 up;
    double fovDegrees;
    double aspectRatio;

    Camera()
        : position(0.0, 0.0, 0.0),
          lookAt(0.0, 0.0, -1.0),
          up(0.0, 1.0, 0.0),
          fovDegrees(60.0),
          aspectRatio(16.0 / 9.0) {
        updateBasis();
    }

    Camera(const Vec3& cameraPosition, const Vec3& target, const Vec3& upVector,
           double fov, double aspect)
        : position(cameraPosition),
          lookAt(target),
          up(upVector),
          fovDegrees(fov),
          aspectRatio(aspect) {
        updateBasis();
    }

    void updateBasis() {
        forward = (lookAt - position).normalized();
        right = cross(forward, up).normalized();
        trueUp = cross(right, forward).normalized();
    }

    Ray generateRay(int pixelX, int pixelY, int width, int height) const {
        constexpr double pi = 3.14159265358979323846;
        double fovRadians = fovDegrees * pi / 180.0;
        double halfHeight = std::tan(fovRadians * 0.5);
        double halfWidth = aspectRatio * halfHeight;

        double u = (2.0 * ((static_cast<double>(pixelX) + 0.5) / width) - 1.0) * halfWidth;
        double v = (1.0 - 2.0 * ((static_cast<double>(pixelY) + 0.5) / height)) * halfHeight;

        Vec3 direction = (forward + right * u + trueUp * v).normalized();
        return Ray(position, direction);
    }

private:
    Vec3 forward;
    Vec3 right;
    Vec3 trueUp;
};

#endif
