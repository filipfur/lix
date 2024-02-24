#include "gluniformbuffer.h"

#include <cassert>
#include "glm.hpp"

lix::UniformBuffer::UniformBuffer(GLuint size, void* data, const std::string& label, GLuint bindingPoint, GLuint usage)
    : Buffer{GL_UNIFORM_BUFFER, usage}, _size{size}, _data{data}, _label{label}, _bindingPoint{bindingPoint}
{
    assert(_size % static_cast<int>(sizeof(glm::vec4)) == 0);
    bufferData();
}

void lix::UniformBuffer::uniformBlockBinding(lix::Element* shader)
{
    GLuint index = glGetUniformBlockIndex(shader->id(), _label.c_str());
    glUniformBlockBinding(shader->id(), index, _bindingPoint);
}

void lix::UniformBuffer::bindBufferBase()
{
    glBindBufferBase(GL_UNIFORM_BUFFER, _bindingPoint, id());
}

void lix::UniformBuffer::bindShaders(const std::vector<lix::Element*>& shaders)
{
    bind();
    for(lix::Element* sp : shaders)
    {
        uniformBlockBinding(sp);
    }
    bindBufferBase();
    unbind();
}

void lix::UniformBuffer::bufferData()
{
    bind();
    Buffer::bufferData(_size, 1, _data);
    unbind();
}

void lix::UniformBuffer::bufferSubData(GLintptr offset, GLsizeiptr size)
{
    bind();
    Buffer::bufferSubData(offset, size, (void*)((char*)_data + offset));
    unbind();
}