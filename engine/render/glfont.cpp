#include "glfont.h"

lix::Font::Font(std::shared_ptr<ttf::Font> font) : _font{font}, _texture{
    font->imageData.data(),
    font->imageWidth,
    font->imageHeight,
    GL_UNSIGNED_BYTE,
    static_cast<GLenum>((font->imageChannels == 3) ? GL_RGB : GL_RGBA),
    static_cast<GLenum>((font->imageChannels == 3) ? GL_RGB : GL_RGBA)
}
{
    _texture.bind();
    _texture.setFilter(GL_LINEAR);
    _texture.unbind();
    for(const auto& character : font->characters)
    {
        _maxCharacterHeight = std::max(_maxCharacterHeight, character.second.height);
    }
}

lix::Texture& lix::Font::texture()
{
    return _texture;
}

const ttf::Character& lix::Font::character(char c)
{
    return _font->characters.at(c);
}

int lix::Font::maxCharacterHeight() const
{
    return _maxCharacterHeight;
}

float lix::Font::width() const
{
    return _font->width;
}

float lix::Font::height() const
{
    return _font->height;
}