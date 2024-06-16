#pragma once

#include "glelement.h"

#include <vector>

namespace lix
{
    class Buffer : public Element
    {
    public:
        Buffer(GLenum target, GLenum usage);
        Buffer(const Buffer& other);
        virtual ~Buffer() noexcept;
    
        virtual Buffer* bind() override;
        virtual Buffer* unbind() override;
    
        void bufferData(GLuint byteLength, GLuint componentSize, void* data=0);
        void bufferData(GLuint byteLength, GLuint componentSize, const void* data);
        void bufferData(const std::vector<GLuint>& data);
        void bufferData(const std::vector<GLushort>& data);
        void bufferData(const std::vector<GLfloat>& data);

        void bufferSubData(GLintptr offset, GLsizeiptr size, void* data);

        GLenum target() const;
        GLenum usage() const;
        GLuint byteLength() const;
        GLenum type() const;
        GLuint count() const;
    private:
        const GLenum _target{GL_ARRAY_BUFFER};
        const GLenum _usage{GL_STATIC_DRAW};
        GLenum _type{GL_FLOAT};
        GLuint _count{0};
        GLuint _byteLength{0};
    };
}