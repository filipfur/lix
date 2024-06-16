#include "glbuffer.h"

lix::Buffer::Buffer(GLenum target, GLenum usage) : _target{target}, _usage{usage}
{
    glGenBuffers(1, &_id);
}
 
lix::Buffer::Buffer(const Buffer& other) : Buffer{other._target, other._usage}
{
    bind();
    bufferData(other._byteLength, 1);
    glBindBuffer(GL_COPY_READ_BUFFER, other._id);
    glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _byteLength);
    _count = other._count;
    _type = other._type;
}
 
lix::Buffer::~Buffer() noexcept
{
    glDeleteBuffers(1, &_id);
}

lix::Buffer* lix::Buffer::bind()
{
    glBindBuffer(_target, _id);
    return this;
}
 
lix::Buffer* lix::Buffer::unbind()
{
    glBindBuffer(_target, 0);
    return this;
}
 
void lix::Buffer::bufferData(GLuint byteLength, GLuint componentSize, void* data)
{
    _byteLength = byteLength;
    _count = byteLength / componentSize;
    glBufferData(_target, _byteLength, data, _usage);
}

void lix::Buffer::bufferData(GLuint byteLength, GLuint componentSize, const void* data)
{
    _byteLength = byteLength;
    _count = byteLength / componentSize;
    glBufferData(_target, _byteLength, data, _usage);
}
 
void lix::Buffer::bufferData(const std::vector<GLuint>& data)
{
    static size_t uintSz = sizeof(GLuint);
    _count = static_cast<GLuint>(data.size());
    _byteLength = static_cast<GLuint>(data.size() * uintSz);
    glBufferData(_target, _byteLength, data.data(), _usage);
    _type = GL_UNSIGNED_INT;
}
 
void lix::Buffer::bufferData(const std::vector<GLushort>& data)
{
    static size_t ushortSz = sizeof(GLushort);
    _count = static_cast<GLuint>(data.size());
    _byteLength = static_cast<GLuint>(data.size() * ushortSz);
    glBufferData(_target, _byteLength, data.data(), _usage);
    _type = GL_UNSIGNED_SHORT;
}

void lix::Buffer::bufferData(const std::vector<GLfloat>& data)
{
    static size_t floatSz = sizeof(GLfloat);
    _count = static_cast<GLuint>(data.size());
    _byteLength = static_cast<GLuint>(data.size() * floatSz);
    glBufferData(_target, _byteLength, data.data(), _usage);
    _type = GL_FLOAT;
}

void lix::Buffer::bufferSubData(GLintptr offset, GLsizeiptr size, void* data)
{
    glBufferSubData(_target, offset, size, data);
}

GLenum lix::Buffer::target() const { return _target; }
GLenum lix::Buffer::usage() const { return _usage; }
GLuint lix::Buffer::byteLength() const { return _byteLength; }
GLenum lix::Buffer::type() const { return _type; }
GLuint lix::Buffer::count() const { return _count; }