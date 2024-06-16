#include "glvertexarray.h"

lix::VertexArray::VertexArray(GLenum mode) : _mode{mode}
{
    glGenVertexArrays(1, &_id);
}

lix::VertexArray::~VertexArray() noexcept
{
    glDeleteVertexArrays(1, &_id);
    _vbos.clear();
}

std::shared_ptr<lix::VBO> lix::VertexArray::vbo(size_t index) const { return _vbos.at(index); }

std::shared_ptr<lix::EBO> lix::VertexArray::ebo() const { return _ebo; }

const std::vector<std::shared_ptr<lix::VBO>>& lix::VertexArray::vbos()
{
    return _vbos;
}

lix::VertexArray::VertexArray(const Attributes& attributes,
    const std::vector<GLfloat>& vertices,
    GLenum mode,
    GLenum usage) : VertexArray{mode}
{
    bind();
    createVbo(usage, attributes, vertices);
}

lix::VertexArray::VertexArray(const Attributes& attributes,
            const std::vector<GLfloat>& vertices,
            const std::vector<GLuint>& indices,
            GLenum mode,
            GLenum usage) : VertexArray{mode}
{
    bind();
    createVbo(usage, attributes, vertices);
    createEbo(usage, indices);
}

lix::VertexArray::VertexArray(const VertexArray& other) : VertexArray{other._mode}
{
    bind();
    for(auto vbo : other._vbos)
    {
        _vbos.push_back(std::make_shared<lix::VertexArrayBuffer>(*vbo));
    }
    if(other._ebo) _ebo = std::make_shared<lix::Buffer>(*other._ebo);
}

lix::VertexArray* lix::VertexArray::bind()
{
    glBindVertexArray(_id);
    return this;
}

lix::VertexArray* lix::VertexArray::unbind()
{
    glBindVertexArray(0);
    return this;
}

std::shared_ptr<lix::VertexArrayBuffer> lix::VertexArray::createVbo(GLenum usage,
            const lix::Attributes& attributes,
            void* data,
            GLuint byteLength,
            int attribDivisor)
{
    int layoutOffset{0};
    for(auto vao : _vbos)
    {
        layoutOffset += vao->layouts();
    }
    auto vbo = std::make_shared<lix::VertexArrayBuffer>(usage,
        attributes,
        data,
        byteLength,
        layoutOffset,
        attribDivisor);
    _vbos.push_back(vbo);
    return vbo;
}

std::shared_ptr<lix::VertexArrayBuffer> lix::VertexArray::createVbo(GLenum usage,
    const lix::Attributes& attributes,
    const std::vector<GLfloat>& vertices,
    int attribDivisor)
{
    int layoutOffset{0};
    for(auto vao : _vbos)
    {
        layoutOffset += vao->layouts();
    }
    auto vbo = std::make_shared<lix::VertexArrayBuffer>(usage,
        attributes,
        vertices,
        layoutOffset,
        attribDivisor);
    _vbos.push_back(vbo);
    return vbo;
}

std::shared_ptr<lix::Buffer> lix::VertexArray::createEbo(GLenum usage,
    const std::vector<GLuint>& indices)
{
    _ebo = std::make_shared<lix::Buffer>(GL_ELEMENT_ARRAY_BUFFER, usage);
    _ebo->bind();
    _ebo->bufferData(indices);
    return _ebo;
}

std::shared_ptr<lix::Buffer> lix::VertexArray::createEbo(GLenum usage,
    const std::vector<GLushort>& indices)
{
    _ebo = std::make_shared<lix::Buffer>(GL_ELEMENT_ARRAY_BUFFER, usage);
    _ebo->bind();
    _ebo->bufferData(indices);
    return _ebo;
}

void lix::VertexArray::draw() const
{
    if(_ebo)
    {
        glDrawElements(_mode, static_cast<GLuint>(_ebo->count()), _ebo->type(), 0);
    }
    else
    {
        int n;
        switch(_mode)
        {
        case GL_TRIANGLES:
            n = _vbos.at(0)->count() / _vbos.at(0)->components(); // TODO: Oversight
            glDrawArrays(_mode, 0, n);
            break;
        default:
            glDrawArrays(_mode, 0, _vbos.at(0)->count());
            break;
        }
    }
}

void lix::VertexArray::drawInstanced(size_t instanceCount) const
{
    glDrawElementsInstanced(_mode, static_cast<GLuint>(_ebo->count()), _ebo->type(), 0, static_cast<GLsizei>(instanceCount));
}