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
    
        void bufferData(GLenum type, GLuint byteLength, GLuint stride, void* data=0);
        void bufferData(const std::vector<GLuint>& data);
        void bufferData(const std::vector<GLushort>& data);
        void bufferData(const std::vector<GLfloat>& data);

        void bufferSubData(GLintptr offset, GLsizeiptr size, void* data);

        GLenum target() const;
        GLenum usage() const;
        GLuint byteLength() const;
        GLuint stride() const;
        GLenum type() const;
        GLuint count() const;
    private:
        void bufferDataInternal(GLuint byteLength, GLuint stride, void* data=0);
        const GLenum _target{GL_ARRAY_BUFFER};
        const GLenum _usage{GL_STATIC_DRAW};
        GLuint _byteLength{0};
        GLuint _stride{0};
        GLuint _count{0};
        GLenum _type{GL_FLOAT};
    };
}