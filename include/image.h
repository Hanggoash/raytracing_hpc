#ifndef RAYTRACING_HPC_IMAGE_H
#define RAYTRACING_HPC_IMAGE_H

#include <string>
#include <vector>

#include "vec3.h"

class Image {
public:
    Image(int imageWidth, int imageHeight);

    int width() const;
    int height() const;
    void setPixel(int x, int y, const Vec3& color);
    const Vec3& getPixel(int x, int y) const;
    bool savePPM(const std::string& filename) const;

private:
    int width_;
    int height_;
    std::vector<Vec3> pixels_;
};

#endif
