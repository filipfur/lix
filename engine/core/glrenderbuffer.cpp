#include "glrenderbuffer.h"

lix::RenderBuffer::RenderBuffer() {
    glGenRenderbuffers(1, &_id);
    bind();
    unbind();
}

void lix::RenderBuffer::storage(glm::ivec2 resolution, GLenum internalFormat) {
    glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, resolution.x,
                          resolution.y);
}

void lix::RenderBuffer::storageMultisample(glm::ivec2 resolution,
                                           GLenum internalFormat,
                                           GLsizei samples) {
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat,
                                     resolution.x, resolution.y);
}

lix::RenderBuffer::~RenderBuffer() noexcept { glDeleteRenderbuffers(1, &_id); }

lix::RenderBuffer *lix::RenderBuffer::bind() {
    glBindRenderbuffer(GL_RENDERBUFFER, _id);
    return this;
}

lix::RenderBuffer *lix::RenderBuffer::unbind() {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return this;
}