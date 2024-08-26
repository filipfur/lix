#include "glfont.h"

lix::Font::Font(const ttf::Font &font)
    : _font{font},
      _texture{
          font.imageData,
          font.imageWidth,
          font.imageHeight,
          GL_UNSIGNED_BYTE,
          static_cast<GLenum>((font.imageChannels == 3) ? GL_RGB : GL_RGBA),
          static_cast<GLenum>((font.imageChannels == 3) ? GL_RGB : GL_RGBA)} {
    _texture.bind();
    _texture.setFilter(GL_LINEAR);
    _texture.unbind();
    for (int i{0}; i <= 94; ++i) {
        _maxCharacterHeight =
            std::max(_maxCharacterHeight, font.characters[i].height);
    }
}

lix::Texture &lix::Font::texture() { return _texture; }

const ttf::Character &lix::Font::character(char c) {
    int index = (int)c - 32;
    assert(index >= 0 && index <= 94);
    return _font.characters[index];
}

float lix::Font::maxCharacterHeight() const { return _maxCharacterHeight; }

unsigned int lix::Font::width() const { return _font.width; }

unsigned int lix::Font::height() const { return _font.height; }