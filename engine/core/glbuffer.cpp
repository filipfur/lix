#include "glbuffer.h"

#include <cassert>

lix::Buffer::Buffer(GLenum target, GLenum usage)
    : _target{target}, _usage{usage} {
    glGenBuffers(1, &_id);
}

lix::Buffer::Buffer(const Buffer &other) : Buffer{other._target, other._usage} {
    bind();
    bufferData(other._type, other._byteLength, other._stride);
    glBindBuffer(GL_COPY_READ_BUFFER, other._id);
    glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                        _byteLength);
}

lix::Buffer::~Buffer() noexcept { glDeleteBuffers(1, &_id); }

lix::Buffer *lix::Buffer::bind() {
    glBindBuffer(_target, _id);
    return this;
}

lix::Buffer *lix::Buffer::unbind() {
    glBindBuffer(_target, 0);
    return this;
}

void lix::Buffer::bufferDataInternal(GLuint byteLength, GLuint stride,
                                     void *data) {
    assert(stride != 0);
    _byteLength = byteLength;
    _stride = stride;
    _count = byteLength / stride;
    glBufferData(_target, _byteLength, data, _usage);
}

void lix::Buffer::bufferData(GLenum type, GLuint byteLength, GLuint stride,
                             void *data) {
    bufferDataInternal(byteLength, stride, data);
    _type = type;
}

void lix::Buffer::bufferData(const std::vector<GLuint> &data) {
    static size_t stride = sizeof(GLuint);
    bufferDataInternal(static_cast<GLuint>(data.size() * stride),
                       static_cast<GLuint>(stride), (void *)data.data());
    _type = GL_UNSIGNED_INT;
}

void lix::Buffer::bufferData(const std::vector<GLushort> &data) {
    static size_t stride = sizeof(GLushort);
    bufferDataInternal(static_cast<GLuint>(data.size() * stride),
                       static_cast<GLuint>(stride), (void *)data.data());
    _type = GL_UNSIGNED_SHORT;
}

void lix::Buffer::bufferData(const std::vector<GLfloat> &data) {
    static size_t stride = sizeof(GLfloat);
    bufferDataInternal(static_cast<GLuint>(data.size() * stride),
                       static_cast<GLuint>(stride), (void *)data.data());
    _type = GL_FLOAT;
}

void lix::Buffer::bufferSubData(GLintptr offset, GLsizeiptr size, void *data) {
    glBufferSubData(_target, offset, size, data);
}

GLenum lix::Buffer::target() const { return _target; }
GLenum lix::Buffer::usage() const { return _usage; }
GLuint lix::Buffer::byteLength() const { return _byteLength; }
GLuint lix::Buffer::stride() const { return _stride; }
GLuint lix::Buffer::count() const { return _count; }
GLenum lix::Buffer::type() const { return _type; }