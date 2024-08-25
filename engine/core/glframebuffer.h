#pragma once

#include "glelement.h"
#include "glrenderbuffer.h"
#include "gltexture.h"
#include <map>
#include <memory>

namespace lix {
class FrameBuffer : public Element {
  public:
    FrameBuffer(const glm::ivec2 &resolution);
    virtual ~FrameBuffer() noexcept;

    virtual FrameBuffer *bind() override;

    virtual FrameBuffer *unbind() override;

    void createRenderBuffer(GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT,
                            GLenum internalFormat = GL_DEPTH24_STENCIL8);
    void createRenderBufferMultisample(GLenum attachment = GL_COLOR_ATTACHMENT0,
                                       GLenum internalFormat = GL_RGBA8,
                                       GLsizei samples = 4);

    void attach(GLenum attachment, std::shared_ptr<RenderBuffer> renderBuffer);

    std::shared_ptr<lix::Texture>
    createTexture(GLuint colorAttachment = GL_COLOR_ATTACHMENT0,
                  GLuint internalFormat = GL_RGB, GLuint format = GL_RGB,
                  GLuint type = GL_UNSIGNED_BYTE);

    void bindTexture(GLuint colorAttachment, GLuint textureUnit);

    void bindColorsAsDrawBuffers();

    void bindAsReadBuffer();

    void bindAsDrawBuffer();

    void checkStatus();

    void readPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                    GLenum format, GLenum type, void *data);

    void readPixel(GLint x, GLint y, GLenum format, GLenum type, void *data);
    glm::vec4 readPixel(GLint x, GLint y);

    std::shared_ptr<lix::Texture> texture(GLuint colorAttachment);

    glm::ivec2 resolution() const;

    std::shared_ptr<lix::RenderBuffer> renderBuffer(size_t i = 0) const;

    void blit(std::shared_ptr<lix::FrameBuffer> frameBuffer,
              GLuint fromComponent, GLuint toComponent,
              GLbitfield mask = GL_COLOR_BUFFER_BIT,
              GLenum filter = GL_NEAREST);

    void blit(std::shared_ptr<lix::FrameBuffer> frameBuffer,
              const glm::ivec2 &resolution, GLuint fromComponent,
              GLuint toComponent, GLbitfield mask = GL_COLOR_BUFFER_BIT,
              GLenum filter = GL_NEAREST);

    static const char *getFramebufferStatusString(GLenum status);

  private:
    static FrameBuffer *_bound;
    glm::ivec2 _resolution;
    std::map<GLuint, std::shared_ptr<lix::Texture>> _textures;
    std::map<GLuint, std::shared_ptr<lix::RenderBuffer>> _renderBuffers;
};

using FBO = lix::FrameBuffer;
} // namespace lix