#pragma once

#include "common.h"

namespace imageproc {
struct ImageData {
    int width;
    int height;
    int channels;
};

unsigned char *loadImage(const fs::path &imageFile, bool flipY, int &width,
                         int &height, int &channels);
void freeImage(unsigned char *data);
void procImage(fs::path inputDir, fs::path outputDir, bool flipOnLoad,
               bool convertToSrgb);
} // namespace imageproc