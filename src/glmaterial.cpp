#include "glmaterial.h"

lix::Material::Material(const lix::Color& baseColor, float metallic, float roughness) : _baseColor{baseColor}, _metallic{metallic}, _roughness{roughness} {}

lix::Material::Material(const Material& other) : _baseColor{other._baseColor}, _metallic{other._metallic}, _roughness{other._roughness},
    _normalMap{other._normalMap}, _diffuseMap{other._diffuseMap}, _armMap{other._armMap}
{
}

std::shared_ptr<lix::Material> lix::Material::Basic(const lix::Color& color)
{
    static std::shared_ptr<lix::Texture> whiteTex = lix::Texture::Basic(lix::Color::white);
    std::shared_ptr<lix::Material> whiteMat{new lix::Material{
            color, 0.0f, 0.5f
    }};
    whiteMat->setDiffuseMap(whiteTex);
    return whiteMat;
}

lix::Color lix::Material::baseColor() const
{
    return _baseColor;
}

float lix::Material::metallic() const
{
    return _metallic;
}

float lix::Material::roughness() const
{
    return _roughness;
}

void lix::Material::setBaseColor(const lix::Color& baseColor)
{
    _baseColor = baseColor;
}

void lix::Material::setMetallic(float metallic)
{
    _metallic = metallic;
}

void lix::Material::setRoughness(float roughness)
{
    _roughness = roughness;
}

void lix::Material::setNormalMap(lix::TexturePtr normalMap)
{
    _normalMap = normalMap;
}

void lix::Material::setDiffuseMap(lix::TexturePtr diffuseMap)
{
    _diffuseMap = diffuseMap;
}

void lix::Material::setArmMap(lix::TexturePtr armMap)
{
    _armMap = armMap;
}

lix::TexturePtr lix::Material::normalMap() const
{
    return _normalMap;
}

lix::TexturePtr lix::Material::diffuseMap() const
{
    return _diffuseMap;
}

lix::TexturePtr lix::Material::armMap() const
{
    return _armMap;
}