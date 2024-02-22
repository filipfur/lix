#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace ttf
{
    struct Character
    {
        char token;
        float x;
        float y;
        float width;
        float height;
        float originX;
        float originY;
        float advance;
    };

    struct Font
    {
        std::string name;
        unsigned int size;
        unsigned int width;
        unsigned int height;
        std::unordered_map<char, Character> characters;
        std::vector<unsigned char> imageData;
        unsigned int imageWidth;
        unsigned int imageHeight;
        unsigned int imageChannels;
    };
}