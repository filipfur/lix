#include "glframebuffer.h"
#include <iostream>
#include <vector>
#include "glm/gtc/type_ptr.hpp"

#include "gltexture.h"

lix::FrameBuffer::FrameBuffer(const glm::ivec2& resolution) 
    : _resolution{resolution}
{
    glGenFramebuffers(1, &_id);
}

lix::FrameBuffer::~FrameBuffer() noexcept
{
    glDeleteFramebuffers(1, &_id);
}

lix::FrameBuffer* lix::FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    return this;
}

lix::FrameBuffer* lix::FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return this;
}

void lix::FrameBuffer::attach(GLenum attachment, std::shared_ptr<RenderBuffer> renderBuffer)
{
    _renderBuffers.emplace(attachment, renderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer->id());
    checkStatus();
}

void lix::FrameBuffer::createRenderBuffer(GLenum attachment, GLenum internalFormat)
{
    auto renderBuffer = std::make_shared<lix::RenderBuffer>();
    renderBuffer->bind();
    renderBuffer->storage(_resolution, internalFormat);
    renderBuffer->unbind();
    attach(attachment, renderBuffer);
}

void lix::FrameBuffer::createRenderBufferMultisample(GLenum attachment, GLenum internalFormat, GLsizei samples)
{
    auto renderBuffer = std::make_shared<lix::RenderBuffer>();
    renderBuffer->bind();
    renderBuffer->storageMultisample(_resolution, internalFormat, samples);
    renderBuffer->unbind();
    attach(attachment, renderBuffer);
}

void lix::FrameBuffer::readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data)
{
    glReadPixels(x, y, width, height, format, type, data);
}

void lix::FrameBuffer::readPixel(GLint x, GLint y, GLenum format, GLenum type, void* data)
{
    readPixels(x, y, 1, 1, format, type, data);
}

glm::vec4 lix::FrameBuffer::readPixel(GLint x, GLint y)
{
    float fp[4]; 
    readPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, fp);
    return glm::vec4(fp[0], fp[1], fp[2], fp[3]);
}

void lix::FrameBuffer::bindAsReadBuffer()
{
    glBindFramebuffer( GL_READ_FRAMEBUFFER, id() );
}

void lix::FrameBuffer::bindAsDrawBuffer()
{
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, id() );
}

std::shared_ptr<lix::Texture> lix::FrameBuffer::texture(GLuint colorAttachment)
{
    return _textures.at(colorAttachment);
}

glm::ivec2 lix::FrameBuffer::resolution() const
{
    return _resolution;
}

std::shared_ptr<lix::RenderBuffer> lix::FrameBuffer::renderBuffer(size_t i) const
{
    return _renderBuffers.at(static_cast<GLuint>(i));
}

void lix::FrameBuffer::blit(std::shared_ptr<lix::FrameBuffer> toFrameBuffer,
    GLuint fromComponent, GLuint toComponent, GLbitfield mask, GLenum filter)
{
    blit(toFrameBuffer, _resolution, fromComponent, toComponent, mask, filter);
}

void lix::FrameBuffer::blit(std::shared_ptr<lix::FrameBuffer> toFrameBuffer, const glm::ivec2& resolution,
    GLuint fromComponent, GLuint toComponent, GLbitfield mask, GLenum filter)
{
    bindAsReadBuffer();
    if(toFrameBuffer)
    {
        toFrameBuffer->bindAsDrawBuffer();
    }
    else
    {
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
    }
    glReadBuffer( fromComponent );
    GLenum arr[] = { toComponent };
    glDrawBuffers(1, arr);
    glBlitFramebuffer( 0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, mask, filter );
}

void lix::FrameBuffer::bindTexture(GLuint colorAttachment, GLuint textureUnit)
{
    auto it = _textures.find(colorAttachment);
    if(it != _textures.end())
    {
        //glBindTexture(it->second->textureMode(), it->second->id());
        it->second->bind(textureUnit);
    }
    else
    {
        std::cerr << "No texture id found for color attachment: " << colorAttachment << std::endl;
    }
}

std::shared_ptr<lix::Texture> lix::FrameBuffer::createTexture(GLuint colorAttachment, GLuint internalFormat, GLuint format, GLuint type)
{
    std::shared_ptr<lix::Texture> tex = std::make_shared<Texture>(_resolution.x, _resolution.y, type, internalFormat, format);
    glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, tex->id(), 0);
    checkStatus();
    _textures[colorAttachment] = tex;
    return tex;
}

void lix::FrameBuffer::bindColorsAsDrawBuffers()
{
    std::vector<GLuint> attachments;
    for(auto it=_textures.begin(); it != _textures.end(); ++it)
    {
        if(it->first >= GL_COLOR_ATTACHMENT0 && it->first <= GL_COLOR_ATTACHMENT15)
        {
            attachments.push_back(it->first);
        }
    }
    for(auto it=_renderBuffers.begin(); it != _renderBuffers.end(); ++it)
    {
        if(it->first >= GL_COLOR_ATTACHMENT0 && it->first <= GL_COLOR_ATTACHMENT15)
        {
            attachments.push_back(it->first);
        }
    }
    glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
    checkStatus();
}


const char* lix::FrameBuffer::getFramebufferStatusString(GLenum status) {
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED: The framebuffer object is not complete because it has no defined buffers.";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: The framebuffer object is not complete because at least one of its buffer attachments is incomplete.";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: The framebuffer object is not complete because it has no attached buffers.";
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: The framebuffer object is not complete because the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_DRAW_BUFFERi.";
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: The framebuffer object is not complete because the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED: The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES, and at least one of the attachments is a texture.";
        default:
            return "Unknown framebuffer status";
    }
}

void lix::FrameBuffer::checkStatus()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error: FBO " << getFramebufferStatusString(status) << std::endl;
    }
}