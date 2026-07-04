#include "scene_generator.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

#include "plane.h"
#include "sphere.h"

namespace {

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

double randomDouble(std::mt19937& rng, double low, double high) {
    std::uniform_real_distribution<double> dist(low, high);
    return dist(rng);
}

Material randomMaterial(std::mt19937& rng, int index) {
    double hueShift = static_cast<double>((index * 37) % 100) / 100.0;
    Vec3 color(
        0.22 + 0.70 * std::fabs(std::sin(6.28318530718 * (hueShift + randomDouble(rng, 0.00, 0.12)))),
        0.22 + 0.70 * std::fabs(std::sin(6.28318530718 * (hueShift + randomDouble(rng, 0.22, 0.34)))),
        0.22 + 0.70 * std::fabs(std::sin(6.28318530718 * (hueShift + randomDouble(rng, 0.48, 0.62))))
    );

    double diffuse = randomDouble(rng, 0.68, 0.90);
    double specular = randomDouble(rng, 0.18, 0.58);
    double shininess = randomDouble(rng, 28.0, 110.0);
    double reflection = (index % 7 == 0) ? randomDouble(rng, 0.18, 0.42) : randomDouble(rng, 0.00, 0.12);
    return Material(color, diffuse, specular, shininess, reflection);
}

void addFixedBaseScene(Renderer& renderer) {
    Material ground(Vec3(0.58, 0.60, 0.56), 0.86, 0.10, 24.0, 0.04);
    Material red(Vec3(0.94, 0.20, 0.18), 0.82, 0.35, 64.0, 0.10);
    Material blue(Vec3(0.16, 0.38, 0.92), 0.78, 0.45, 80.0, 0.16);
    Material green(Vec3(0.22, 0.72, 0.35), 0.82, 0.25, 48.0, 0.06);
    Material mirror(Vec3(0.90, 0.92, 0.96), 0.35, 0.95, 160.0, 0.58);
    Material gold(Vec3(1.00, 0.70, 0.25), 0.72, 0.65, 96.0, 0.22);

    renderer.addObject(std::make_shared<Plane>(Vec3(0.0, -1.0, 0.0), Vec3(0.0, 1.0, 0.0), ground));
    renderer.addObject(std::make_shared<Sphere>(Vec3(-1.55, -0.15, -0.75), 0.85, red));
    renderer.addObject(std::make_shared<Sphere>(Vec3(0.15, -0.35, -1.85), 0.65, mirror));
    renderer.addObject(std::make_shared<Sphere>(Vec3(1.35, -0.10, -0.60), 0.90, blue));
    renderer.addObject(std::make_shared<Sphere>(Vec3(-0.30, 0.65, -2.60), 0.45, green));
    renderer.addObject(std::make_shared<Sphere>(Vec3(1.95, -0.55, -2.20), 0.45, gold));
}

bool overlapsFixedHeroSphere(const Vec3& center, double radius) {
    const Vec3 fixedCenters[] = {
        Vec3(-1.55, -0.15, -0.75),
        Vec3(0.15, -0.35, -1.85),
        Vec3(1.35, -0.10, -0.60),
        Vec3(-0.30, 0.65, -2.60),
        Vec3(1.95, -0.55, -2.20)
    };
    const double fixedRadii[] = {0.85, 0.65, 0.90, 0.45, 0.45};

    for (int i = 0; i < 5; ++i) {
        double minDistance = radius + fixedRadii[i] + 0.18;
        if ((center - fixedCenters[i]).length() < minDistance) {
            return true;
        }
    }
    return false;
}

void addRandomSpheres(Renderer& renderer, SceneSize sceneSize, int sceneId, unsigned int baseSeed) {
    std::mt19937 rng(baseSeed + static_cast<unsigned int>(sceneId) * 1009U);
    int targetCount = randomSphereCount(sceneSize);
    int created = 0;
    int attempts = 0;

    while (created < targetCount && attempts < targetCount * 60) {
        ++attempts;
        double radius = randomDouble(rng, 0.12, sceneSize == SceneSize::Large ? 0.38 : 0.32);
        double x = randomDouble(rng, -5.0, 5.0);
        double z = randomDouble(rng, -5.8, -0.7);
        double y = -1.0 + radius;
        Vec3 center(x, y, z);

        if (overlapsFixedHeroSphere(center, radius)) {
            continue;
        }

        renderer.addObject(std::make_shared<Sphere>(center, radius, randomMaterial(rng, created + sceneId)));
        ++created;
    }
}

void addSceneLights(Renderer& renderer, std::mt19937& rng, int sceneId) {
    renderer.addLight(Light(Vec3(-3.4, 4.8, 3.2), Vec3(1.0, 0.96, 0.90), 4.2));
    renderer.addLight(Light(Vec3(3.6, 3.2, 2.8), Vec3(0.65, 0.78, 1.0), 1.8));

    double x = randomDouble(rng, -2.8, 2.8);
    double z = randomDouble(rng, -3.8, 0.6);
    double intensity = 0.8 + 0.2 * static_cast<double>(sceneId % 5);
    renderer.addLight(Light(Vec3(x, 2.2 + randomDouble(rng, 0.0, 1.4), z),
                            Vec3(0.80, 0.88, 1.0), intensity));
}

} // namespace

SceneSize parseSceneSize(const std::string& value) {
    std::string lowered = lowerCopy(value);
    if (lowered == "small") {
        return SceneSize::Small;
    }
    if (lowered == "medium") {
        return SceneSize::Medium;
    }
    if (lowered == "large") {
        return SceneSize::Large;
    }
    throw std::invalid_argument("--scene-size must be small, medium, or large.");
}

std::string sceneSizeName(SceneSize size) {
    switch (size) {
        case SceneSize::Small:
            return "small";
        case SceneSize::Medium:
            return "medium";
        case SceneSize::Large:
            return "large";
    }
    return "medium";
}

int randomSphereCount(SceneSize size) {
    switch (size) {
        case SceneSize::Small:
            return 10;
        case SceneSize::Medium:
            return 32;
        case SceneSize::Large:
            return 96;
    }
    return 32;
}

Renderer SceneGenerator::createScene(int imageWidth, int imageHeight, int maxReflectionDepth,
                                     int sceneId, SceneSize sceneSize, unsigned int baseSeed) {
    Renderer renderer(imageWidth, imageHeight, maxReflectionDepth);

    double cameraX = 0.25 * std::sin(sceneId * 0.77);
    renderer.setCamera(Camera(
        Vec3(cameraX, 1.35, 5.5),
        Vec3(0.0, 0.05, -1.25),
        Vec3(0.0, 1.0, 0.0),
        58.0,
        static_cast<double>(imageWidth) / static_cast<double>(imageHeight)
    ));

    addFixedBaseScene(renderer);

    std::mt19937 rng(baseSeed + static_cast<unsigned int>(sceneId) * 1009U);
    addRandomSpheres(renderer, sceneSize, sceneId, baseSeed);
    addSceneLights(renderer, rng, sceneId);

    return renderer;
}
