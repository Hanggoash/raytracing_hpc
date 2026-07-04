#ifndef RAYTRACING_HPC_RENDERER_H
#define RAYTRACING_HPC_RENDERER_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "camera.h"
#include "image.h"
#include "light.h"
#include "object.h"

class Renderer {
public:
    Renderer(int imageWidth, int imageHeight, int maxReflectionDepth = 2);

    static Renderer createDefaultScene(int imageWidth, int imageHeight, int maxReflectionDepth = 2);

    void setCamera(const Camera& newCamera);
    void addObject(const std::shared_ptr<Object>& object);
    void addLight(const Light& light);
    std::size_t objectCount() const;
    std::size_t lightCount() const;

    double serialRender(const std::string& outputPath) const;
    double openmpRender(const std::string& outputPath, int threads, const std::string& schedule) const;

    Vec3 traceRay(const Ray& ray, int depth) const;
    Vec3 shade(const Ray& ray, const HitRecord& hit, int depth) const;
    bool closestHit(const Ray& ray, double tMin, double tMax, HitRecord& closestRecord) const;
    bool isInShadow(const Vec3& point, const Vec3& lightDirection, double lightDistance) const;
    Vec3 backgroundColor(const Ray& ray) const;

private:
    int width_;
    int height_;
    int maxDepth_;
    Camera camera_;
    std::vector<std::shared_ptr<Object>> objects_;
    std::vector<Light> lights_;
    Vec3 backgroundBottom_;
    Vec3 backgroundTop_;
    double ambientStrength_;
};

#endif
