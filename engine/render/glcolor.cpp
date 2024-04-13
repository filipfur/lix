#include <utility>

#include "glcolor.h"

float byteToDecimal(uint32_t hex, unsigned int pos)
{
    static constexpr float factor{1.0f / 255.0f};
    return static_cast<float>((hex >> pos) & 0xFF) * factor;
}

lix::Color::Color(uint32_t hex, float alpha)
    : _rgba{byteToDecimal(hex, 16), byteToDecimal(hex, 8), byteToDecimal(hex, 0), alpha}
{

}

lix::Color::Color(float r, float g, float b) : _rgba{r, g, b, 1.0f}
{

}

lix::Color::Color(float r, float g, float b, float a) : _rgba{r, g, b, a}
{
    
}

lix::Color::Color(const Color& other) : _rgba{other._rgba}
{
    
}

lix::Color::Color(Color&& other) : _rgba{std::move(other._rgba)}
{
    
}

lix::Color::Color(const glm::vec4& rgba) : _rgba{rgba}
{

}

lix::Color::Color(const glm::vec3& rgb) : _rgba{rgb.x, rgb.y, rgb.z, 1.0f}
{

}

lix::Color& lix::Color::operator=(const lix::Color& other)
{
    _rgba = other._rgba;
    return *this;
}

lix::Color& lix::Color::operator=(lix::Color&& other)
{
    _rgba = std::move(other._rgba);
    return *this;
}

lix::Color::operator glm::vec4() const
{
    return _rgba;
}

const glm::vec4& lix::Color::vec4() const
{
    return _rgba;
}

const lix::Color lix::Color::white{1.0f, 1.0f, 1.0f};
const lix::Color lix::Color::red{1.0f, 0.0f, 0.0f};
const lix::Color lix::Color::green{0.0f, 1.0f, 0.0f};
const lix::Color lix::Color::blue{0.0f, 0.0f, 1.0f};
const lix::Color lix::Color::yellow{1.0f, 1.0f, 0.0f};
const lix::Color lix::Color::magenta{1.0f, 0.0f, 1.0f};
const lix::Color lix::Color::cyan{0.0f, 1.0f, 1.0f};
const lix::Color lix::Color::black{0.0f, 0.0f, 0.0f};