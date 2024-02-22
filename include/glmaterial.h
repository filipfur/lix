#pragma once

#include <memory>
#include <vector>

#include "glm.hpp"

#include "gltexture.h"
#include "glcolor.h"

namespace lix
{
    class Material
    {
    public:
        Material(const Color& baseColor, float metallic=0.0f, float roughness=0.5f);

        Material(const Material& other);

        static std::shared_ptr<lix::Material> Basic(const lix::Color& color);

        Color baseColor() const;

        float metallic() const;

        float roughness() const;

        void setBaseColor(const Color& baseColor);

        void setMetallic(float metallic);

        void setRoughness(float roughness);

        void setNormalMap(TexturePtr normalMap);

        void setDiffuseMap(TexturePtr diffuseMap);

        void setArmMap(TexturePtr armMap);

        TexturePtr normalMap() const;

        TexturePtr diffuseMap() const;

        TexturePtr armMap() const;

    private:
        Color _baseColor{Color::white};
        float _metallic{0.0f};
        float _roughness{0.5f};

        TexturePtr _normalMap{nullptr};
        TexturePtr _diffuseMap{nullptr};
        TexturePtr _armMap{nullptr};
    };
}