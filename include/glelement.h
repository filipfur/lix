#pragma once

#include <GL/glew.h>

namespace lix
{
    class Element
    {
    public:
        virtual ~Element() noexcept = 0;

        GLuint id() const;
        
        virtual Element* bind() = 0;
        virtual Element* unbind() = 0;
        
    protected:
        GLuint _id;
    };
}