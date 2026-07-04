#include "image.h"

#include <fstream>
#include <iostream>

#include "file_utils.h"

Image::Image(int imageWidth, int imageHeight)
    : width_(imageWidth),
      height_(imageHeight),
      pixels_(static_cast<std::size_t>(imageWidth * imageHeight), Vec3(0.0, 0.0, 0.0)) {}

int Image::width() const {
    return width_;
}

int Image::height() const {
    return height_;
}

void Image::setPixel(int x, int y, const Vec3& color) {
    pixels_[static_cast<std::size_t>(y * width_ + x)] = color.clamped();
}

const Vec3& Image::getPixel(int x, int y) const {
    return pixels_[static_cast<std::size_t>(y * width_ + x)];
}

bool Image::savePPM(const std::string& filename) const {
    if (!ensureParentDirectory(filename)) {
        std::cerr << "Failed to create output directory for: " << filename << "\n";
        return false;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open image file for writing: " << filename << "\n";
        return false;
    }

    out << "P6\n" << width_ << " " << height_ << "\n255\n";
    for (const Vec3& color : pixels_) {
        unsigned char rgb[3] = {
            static_cast<unsigned char>(Vec3::toByte(color.x)),
            static_cast<unsigned char>(Vec3::toByte(color.y)),
            static_cast<unsigned char>(Vec3::toByte(color.z))
        };
        out.write(reinterpret_cast<const char*>(rgb), 3);
    }

    return static_cast<bool>(out);
}
