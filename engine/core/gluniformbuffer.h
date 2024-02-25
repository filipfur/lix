#pragma once

#include <string>
#include "glbuffer.h"

namespace lix
{
    class UniformBuffer : public Buffer
    {
    public:
        UniformBuffer(GLuint size, void* data, const std::string& label, GLuint bindingPoint, GLuint usage=GL_STATIC_DRAW);

        void uniformBlockBinding(lix::Element* shader);

        void bindBufferBase();

        void bindShaders(const std::vector<lix::Element*>& shaders);

        void bufferData();

        void bufferSubData(GLintptr offset, GLsizeiptr size);

    private:
        const GLuint _size;
        void* _data{nullptr};
        const std::string _label;
        const GLuint _bindingPoint;
    };
}