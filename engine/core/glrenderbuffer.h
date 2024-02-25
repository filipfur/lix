#pragma once

#include "glelement.h"

#include "glm/glm.hpp"

namespace lix
{
    class RenderBuffer : public Element
    {
    public:
        RenderBuffer();
        virtual ~RenderBuffer() noexcept;

        void storage(glm::ivec2 resolution, GLenum internalFormat);
        void storageMultisample(glm::ivec2 resolution, GLenum internalFormat, GLsizei samples);

        virtual RenderBuffer* bind() override;
        virtual RenderBuffer* unbind() override;
    };
}