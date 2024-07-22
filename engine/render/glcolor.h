#pragma once

#include "glm/glm.hpp"

namespace lix
{
    class Color
    {
    public:
        Color(uint32_t hex, float alpha=1.0f);
        Color(float r, float g, float b);
        Color(float r, float g, float b, float a);
        Color(const Color& color);
        Color(Color&& color);
        Color(const glm::vec3& rgb);
        Color(const glm::vec4& rgba);

        static Color opacity(float val);

        Color& operator=(const Color& color);
        Color& operator=(Color&& color);

        Color operator*(const Color& other) const;

        operator glm::vec4() const;

        const glm::vec4& vec4() const;

        static const Color white;
        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color yellow;
        static const Color magenta;
        static const Color cyan;
        static const Color black;

    private:
        glm::vec4 _rgba{1.0f, 1.0f, 1.0f, 1.0f};
    };
    
}