#pragma once

#include "gltexture.h"

#include "ttf.h"

namespace lix
{
    class Font
    {
    public:
        Font(std::shared_ptr<ttf::Font> font);

        lix::Texture& texture();

        const ttf::Character& character(char c);

        int maxCharacterHeight() const;

        float width() const;

        float height() const;

    private:
        std::shared_ptr<ttf::Font> _font;
        lix::Texture _texture;
        float _maxCharacterHeight{0.0f};
    };
}