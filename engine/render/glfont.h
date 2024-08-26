#pragma once

#include "gltexture.h"
#include <memory>

#include "ttf.h"

namespace lix {
class Font {
  public:
    Font(const ttf::Font &font);

    lix::Texture &texture();

    const ttf::Character &character(char c);

    float maxCharacterHeight() const;

    unsigned int width() const;

    unsigned int height() const;

  private:
    const ttf::Font &_font;
    lix::Texture _texture;
    float _maxCharacterHeight{0.0f};
};
} // namespace lix