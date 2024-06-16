#include "gltexture.h"

#include <stdexcept>

#include "glerror.h"

lix::Texture::Texture(unsigned int width, unsigned int height, GLenum type,
    GLenum internalFormat, GLenum colorFormat)
    : _width{width}, _height{height}, _type{type}, _internalFormat{internalFormat},
        _colorFormat{colorFormat}
{
    glGenTextures(1, &_id);
    static unsigned char* nuller = nullptr;
    bind(_active);
    texImage2D(nuller);
    errorCheck();
    setUnpackAlignment();
    setFilter()->setWrap();
    unbind();
}

lix::Texture::Texture(unsigned char* bytes, unsigned int width, unsigned int height, GLenum type,
    GLenum internalFormat, GLenum colorFormat)
    : Texture{width, height, type, internalFormat, colorFormat}
{
    bind(_active);
    texImage2D(bytes);
    errorCheck();
    setUnpackAlignment();
    setFilter()->setWrap();
    unbind();
}

lix::Texture::Texture(const unsigned char* bytes, unsigned int width, unsigned int height, GLenum type,
    GLenum internalFormat, GLenum colorFormat)
    : Texture{width, height, type, internalFormat, colorFormat}
{
    bind(_active);
    texImage2D(bytes);
    errorCheck();
    setUnpackAlignment();
    setFilter()->setWrap();
    unbind();
}

lix::Texture::~Texture() noexcept
{
    glDeleteTextures(1, &_id);
}

std::shared_ptr<lix::Texture> lix::Texture::Basic(lix::Color color)
{
    unsigned char data[] = {
        static_cast<unsigned char>(255.0f * color.vec4().r),
        static_cast<unsigned char>(255.0f * color.vec4().g),
        static_cast<unsigned char>(255.0f * color.vec4().b)
    };
    return std::make_shared<lix::Texture>(data, 1, 1, GL_UNSIGNED_BYTE, GL_RGB8, GL_RGB);
}

lix::Texture* lix::Texture::setUnpackAlignment(GLuint unpackAlignment)
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
    return this;
}

lix::Texture* lix::Texture::setFilter(GLenum min, GLenum mag)
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    return this;
}

lix::Texture* lix::Texture::setFilter(GLenum filter) { return setFilter(filter, filter); }

lix::Texture* lix::Texture::setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR)
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapR);
    return this;
}

lix::Texture* lix::Texture::setWrap(GLenum wrap) { return setWrap(wrap, wrap, wrap); }

lix::Texture* lix::Texture::generateMipmap()
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glGenerateMipmap(GL_TEXTURE_2D);
    return this;
}

lix::Texture* lix::Texture::setLodBias(float bias)
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, bias);
    return this;
}

lix::Texture* lix::Texture::bind(GLuint textureUnit)
{
    if(textureUnit < GL_TEXTURE0 || textureUnit > GL_TEXTURE31)
    {
        throw std::runtime_error("Error: Invalid texture unit.");
    }
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _id);
    _bound[textureUnit - GL_TEXTURE0] = this;
    _active = textureUnit;
    return this;
}

lix::Texture* lix::Texture::bind()
{
    return bind(_active);
}

lix::Texture* lix::Texture::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    return this;
}

void lix::Texture::texImage2D(unsigned char* bytes)
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, _width, _height, 0, _colorFormat, _type, bytes);
}

void lix::Texture::texImage2D(const unsigned char* bytes)
{
    assert(_bound[_active - GL_TEXTURE0] == this);
    glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, _width, _height, 0, _colorFormat, _type, bytes);
}

void lix::Texture::errorCheck()
{
    static const std::string className{"Texture"};
    lix::traceGLError(className, glGetError());
}

int lix::Texture::width() const
{
    return _width;
}

int lix::Texture::height() const
{
    return _height;
}

GLenum lix::Texture::type() const
{
    return _type;
}

GLenum lix::Texture::internalFormat() const
{
    return _internalFormat;
}

GLenum lix::Texture::colorFormat() const
{
    return _colorFormat;
}

int lix::Texture::_active{GL_TEXTURE0};
lix::Texture* lix::Texture::_bound[32]{};