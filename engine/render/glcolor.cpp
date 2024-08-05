#include <utility>
#include <string>
#include <sstream>
#include <iomanip>

#include "glcolor.h"

static glm::vec3 rgbToHsv(const glm::vec3& rgb) {
    float r = rgb.r, g = rgb.g, b = rgb.b;
    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float delta = max - min;
    float h, s, v;
    v = max; // value
    if (max == 0.0f) {
        s = 0.0f; // saturation
        h = 0.0f; // hue is undefined when saturation is 0
    } else {
        s = delta / max; // saturation
        if (r == max) {
            h = (g - b) / delta; // between yellow & magenta
        } else if (g == max) {
            h = 2.0f + (b - r) / delta; // between cyan & yellow
        } else {
            h = 4.0f + (r - g) / delta; // between magenta & cyan
        }
        h *= 60.0f; // degrees
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
    return {h, s, v};
}

static glm::vec3 hsvToRgb(float h, float s, float v) {
    float r, g, b;

    int i = static_cast<int>(h / 60.0f) % 6;
    float f = (h / 60.0f) - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }

    return {r, g, b};
}

float byteToDecimal(uint32_t hex, unsigned int pos)
{
    static constexpr float factor{1.0f / 255.0f};
    return static_cast<float>((hex >> pos) & 0xFF) * factor;
}

lix::Color::Color(const std::string& hexstr)
    : lix::Color{static_cast<uint32_t>(std::stoi(hexstr, 0, 16))}
{
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

lix::Color lix::Color::opacity(float val)
{
    return lix::Color{1.0f, 1.0f, 1.0f, val};
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

lix::Color lix::Color::operator*(const lix::Color& other) const
{
    return {this->vec4() * other.vec4()};
}

lix::Color::operator glm::vec4() const
{
    return _rgba;
}

glm::vec3 lix::Color::vec3() const
{
    return {_rgba};
}

const glm::vec4& lix::Color::vec4() const
{
    return _rgba;
}

glm::vec4& lix::Color::vec4()
{
    return _rgba;
}

glm::vec3 lix::Color::hsv() const
{
    return rgbToHsv(glm::vec3{_rgba.r, _rgba.g, _rgba.b});
}

lix::Color& lix::Color::setHSV(float h, float s, float v)
{
    glm::vec3 rgb = hsvToRgb(h, s, v);
    _rgba.r = rgb.r;
    _rgba.g = rgb.g;
    _rgba.b = rgb.b;
    return *this;
}

void lix::Color::hueShift(float rad)
{
    float U = cosf(rad);
    float W = sinf(rad);
    static constexpr glm::mat3 yiqMatrix{0.299f, 0.596f, 0.211f, 0.587f, -0.274f, -0.523f, 0.114f, -0.321f, 0.311f};
    glm::mat3 hueMatrix{1.0f, 0.0f, 0.0f, 0.0f, U, W, 0.0f, -W, U};
    static constexpr glm::mat3 rgbMatrix{1.0f, 1.0f, 1.0f, 0.956f, -0.272f, -1.107f, 0.621f, -0.647f, 1.705f};
    glm::mat3 T = rgbMatrix * hueMatrix * yiqMatrix;
    glm::vec3 rgb = T * vec3();
    _rgba.r = std::max(0.0f, std::min(1.0f, rgb.r));
    _rgba.g = std::max(0.0f, std::min(1.0f, rgb.g));
    _rgba.b = std::max(0.0f, std::min(1.0f, rgb.b));
}

void lix::Color::saturate(float S)
{
    static constexpr glm::mat3 yiqMatrix{0.299f, 0.596f, 0.211f, 0.587f, -0.274f, -0.523f, 0.114f, -0.321f, 0.311f};
    glm::mat3 satMatrix{1.0f, 0.0f, 0.0f, 0.0f, S, 0.0f, 0.0f, 0.0f, S};
    static constexpr glm::mat3 rgbMatrix{1.0f, 1.0f, 1.0f, 0.956f, -0.272f, -1.107f, 0.621f, -0.647f, 1.705f};
    glm::mat3 T = rgbMatrix * satMatrix * yiqMatrix;
    glm::vec3 rgb = T * vec3();
    _rgba.r = std::max(0.0f, std::min(1.0f, rgb.r));
    _rgba.g = std::max(0.0f, std::min(1.0f, rgb.g));
    _rgba.b = std::max(0.0f, std::min(1.0f, rgb.b));
}

void lix::Color::valueScale(float V)
{
    static constexpr glm::mat3 yiqMatrix{0.299f, 0.596f, 0.211f, 0.587f, -0.274f, -0.523f, 0.114f, -0.321f, 0.311f};
    glm::mat3 valMatrix{V, 0.0f, 0.0f, 0.0f, V, 0.0f, 0.0f, 0.0f, V};
    static constexpr glm::mat3 rgbMatrix{1.0f, 1.0f, 1.0f, 0.956f, -0.272f, -1.107f, 0.621f, -0.647f, 1.705f};
    glm::mat3 T = rgbMatrix * valMatrix * yiqMatrix;
    glm::vec3 rgb = T * vec3();
    _rgba.r = std::max(0.0f, std::min(1.0f, rgb.r));
    _rgba.g = std::max(0.0f, std::min(1.0f, rgb.g));
    _rgba.b = std::max(0.0f, std::min(1.0f, rgb.b));
}

std::string lix::Color::hexString() const
{
    int ri = static_cast<int>(_rgba.r * 255);
    int gi = static_cast<int>(_rgba.g * 255);
    int bi = static_cast<int>(_rgba.b * 255);

    // Use a stringstream to format as hex
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2) << ri
       << std::setfill('0') << std::setw(2) << gi
       << std::setfill('0') << std::setw(2) << bi;

    // Return the resulting string
    return ss.str();
}

const lix::Color lix::Color::white{1.0f, 1.0f, 1.0f};
const lix::Color lix::Color::red{1.0f, 0.0f, 0.0f};
const lix::Color lix::Color::green{0.0f, 1.0f, 0.0f};
const lix::Color lix::Color::blue{0.0f, 0.0f, 1.0f};
const lix::Color lix::Color::yellow{1.0f, 1.0f, 0.0f};
const lix::Color lix::Color::magenta{1.0f, 0.0f, 1.0f};
const lix::Color lix::Color::cyan{0.0f, 1.0f, 1.0f};
const lix::Color lix::Color::black{0.0f, 0.0f, 0.0f};