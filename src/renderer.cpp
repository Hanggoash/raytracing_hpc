#include "renderer.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "scene_generator.h"

namespace {
constexpr double kEpsilon = 1e-4;
}

Renderer::Renderer(int imageWidth, int imageHeight, int maxReflectionDepth)
    : width_(imageWidth),
      height_(imageHeight),
      maxDepth_(maxReflectionDepth),
      camera_(Vec3(0.0, 1.2, 5.0), Vec3(0.0, 0.2, 0.0), Vec3(0.0, 1.0, 0.0),
              60.0, static_cast<double>(imageWidth) / imageHeight),
      backgroundBottom_(0.72, 0.82, 0.95),
      backgroundTop_(0.08, 0.16, 0.32),
      ambientStrength_(0.12) {}

Renderer Renderer::createDefaultScene(int imageWidth, int imageHeight, int maxReflectionDepth) {
    return SceneGenerator::createScene(imageWidth, imageHeight, maxReflectionDepth,
                                       1, SceneSize::Medium, 20260704U);
}

void Renderer::setCamera(const Camera& newCamera) {
    camera_ = newCamera;
}

void Renderer::addObject(const std::shared_ptr<Object>& object) {
    objects_.push_back(object);
}

void Renderer::addLight(const Light& light) {
    lights_.push_back(light);
}

std::size_t Renderer::objectCount() const {
    return objects_.size();
}

std::size_t Renderer::lightCount() const {
    return lights_.size();
}

double Renderer::serialRender(const std::string& outputPath) const {
    Image image(width_, height_);

    auto start = std::chrono::high_resolution_clock::now();
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Ray ray = camera_.generateRay(x, y, width_, height_);
            image.setPixel(x, y, traceRay(ray, 0));
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (!image.savePPM(outputPath)) {
        throw std::runtime_error("Failed to save image: " + outputPath);
    }

    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

double Renderer::openmpRender(const std::string& outputPath, int threads, const std::string& schedule) const {
    Image image(width_, height_);
    int safeThreads = std::max(1, threads);

    auto start = std::chrono::high_resolution_clock::now();

#ifdef _OPENMP
    omp_set_num_threads(safeThreads);
    if (schedule == "dynamic") {
#pragma omp parallel for schedule(dynamic, 1)
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                Ray ray = camera_.generateRay(x, y, width_, height_);
                image.setPixel(x, y, traceRay(ray, 0));
            }
        }
    } else if (schedule == "static") {
#pragma omp parallel for schedule(static)
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                Ray ray = camera_.generateRay(x, y, width_, height_);
                image.setPixel(x, y, traceRay(ray, 0));
            }
        }
    } else {
        throw std::invalid_argument("OpenMP schedule must be static or dynamic.");
    }
#else
    std::cerr << "OpenMP is not enabled. Falling back to serial rendering.\n";
    (void)safeThreads;
    (void)schedule;
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Ray ray = camera_.generateRay(x, y, width_, height_);
            image.setPixel(x, y, traceRay(ray, 0));
        }
    }
#endif

    auto end = std::chrono::high_resolution_clock::now();

    if (!image.savePPM(outputPath)) {
        throw std::runtime_error("Failed to save image: " + outputPath);
    }

    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

Vec3 Renderer::traceRay(const Ray& ray, int depth) const {
    if (depth > maxDepth_) {
        return backgroundColor(ray);
    }

    HitRecord hit;
    if (!closestHit(ray, kEpsilon, 1e30, hit)) {
        return backgroundColor(ray);
    }

    return shade(ray, hit, depth).clamped();
}

Vec3 Renderer::shade(const Ray& ray, const HitRecord& hit, int depth) const {
    Vec3 color = hit.material.baseColor * ambientStrength_;
    Vec3 viewDirection = (-ray.direction).normalized();

    for (const Light& light : lights_) {
        Vec3 toLight = light.position - hit.position;
        double lightDistance = toLight.length();
        Vec3 lightDirection = toLight / lightDistance;

        if (isInShadow(hit.position + hit.normal * kEpsilon, lightDirection, lightDistance)) {
            continue;
        }

        double distanceAttenuation = light.intensity /
            (1.0 + 0.045 * lightDistance + 0.0075 * lightDistance * lightDistance);
        double diffuseFactor = std::max(0.0, dot(hit.normal, lightDirection));
        Vec3 diffuseColor = multiply(hit.material.baseColor, light.color) *
            (hit.material.diffuse * diffuseFactor * distanceAttenuation);

        Vec3 halfVector = (lightDirection + viewDirection).normalized();
        double specAngle = std::max(0.0, dot(hit.normal, halfVector));
        double specFactor = std::pow(specAngle, hit.material.shininess);
        Vec3 specularColor = light.color *
            (hit.material.specular * specFactor * distanceAttenuation);

        color += diffuseColor + specularColor;
    }

    if (hit.material.reflection > 0.0 && depth < maxDepth_) {
        Vec3 reflectionDirection = reflect(ray.direction, hit.normal).normalized();
        Ray reflectionRay(hit.position + hit.normal * kEpsilon, reflectionDirection);
        Vec3 reflectedColor = traceRay(reflectionRay, depth + 1);
        double reflectionAmount = std::clamp(hit.material.reflection, 0.0, 1.0);
        color = color * (1.0 - reflectionAmount) + reflectedColor * reflectionAmount;
    }

    return color;
}

bool Renderer::closestHit(const Ray& ray, double tMin, double tMax, HitRecord& closestRecord) const {
    HitRecord tempRecord;
    bool hasHit = false;
    double closestSoFar = tMax;

    for (const auto& object : objects_) {
        if (object->intersect(ray, tMin, closestSoFar, tempRecord)) {
            hasHit = true;
            closestSoFar = tempRecord.t;
            closestRecord = tempRecord;
        }
    }

    return hasHit;
}

bool Renderer::isInShadow(const Vec3& point, const Vec3& lightDirection, double lightDistance) const {
    Ray shadowRay(point, lightDirection);
    HitRecord shadowHit;
    return closestHit(shadowRay, kEpsilon, lightDistance - kEpsilon, shadowHit);
}

Vec3 Renderer::backgroundColor(const Ray& ray) const {
    double t = 0.5 * (ray.direction.normalized().y + 1.0);
    return lerp(backgroundBottom_, backgroundTop_, t);
}
