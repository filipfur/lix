#pragma once

#include <vector>

#include "glbuffer.h"

namespace lix
{
    struct AttributePointer
    {
        GLuint componentType;
        GLuint components;
        GLuint elements;
        GLuint size;        
    };

    enum Attribute { FLOAT, VEC2, VEC3, VEC4, UVEC4, MAT3, MAT4 };

    using Attributes = std::vector<lix::Attribute>;

    class VertexArrayBuffer : public Buffer
    {
    public:
        VertexArrayBuffer(GLenum usage,
            const lix::Attributes& attributes,
            GLuint byteLength,
            GLuint componentSize,
            void* data,
            GLuint layoutOffset=0,
            GLuint attribDivisor=0,
            GLuint componentType=GL_UNSIGNED_BYTE);

        VertexArrayBuffer(GLenum usage,
            const lix::Attributes& attributes,
            GLuint byteLength,
            GLuint componentSize,
            const void* data,
            GLuint layoutOffset=0,
            GLuint attribDivisor=0,
            GLuint componentType=GL_UNSIGNED_BYTE);

        VertexArrayBuffer(GLenum usage,
            const lix::Attributes& attributes,
            const std::vector<GLfloat>& vertices,
            GLuint layoutOffset=0,
            GLuint attribDivisor=0);
        VertexArrayBuffer(const VertexArrayBuffer& other);
        virtual ~VertexArrayBuffer() noexcept;

        void linkAttributes();

        GLuint layoutOffset() const;
        GLuint attribDivisor() const;
        GLuint componentType() const;
        GLuint layouts() const;
        GLuint components() const;

    private:
        Attributes _attributes;
		GLuint _layoutOffset;
		GLuint _attribDivisor;
		GLuint _componentType;
        GLuint _layouts{0};
		GLuint _components{0};
    };
}