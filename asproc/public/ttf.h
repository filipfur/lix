#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace ttf {
struct Character {
    char token;
    float x;
    float y;
    float width;
    float height;
    float originX;
    float originY;
    float advance;
};

struct Font {
    const char *name;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    const Character *characters;
    const unsigned char *imageData;
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int imageChannels;
};
} // namespace ttf