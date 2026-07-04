#ifndef RAYTRACING_HPC_SCENE_GENERATOR_H
#define RAYTRACING_HPC_SCENE_GENERATOR_H

#include <string>

#include "renderer.h"

enum class SceneSize {
    Small,
    Medium,
    Large
};

SceneSize parseSceneSize(const std::string& value);
std::string sceneSizeName(SceneSize size);
int randomSphereCount(SceneSize size);

class SceneGenerator {
public:
    static Renderer createScene(int imageWidth, int imageHeight, int maxReflectionDepth,
                                int sceneId, SceneSize sceneSize, unsigned int baseSeed);
};

#endif
