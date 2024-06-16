#pragma once

#include <memory>
#include <vector>

#include "glelement.h"
#include "glvertexarraybuffer.h"

namespace lix
{
    using VBO = VertexArrayBuffer;
    using EBO = Buffer;

    class VertexArray : Element
    {
    public:

        VertexArray(GLenum mode=GL_TRIANGLES);
        VertexArray(const Attributes& attributes,
            const std::vector<GLfloat>& vertices,
            const std::vector<GLuint>& indices,
            GLenum mode=GL_TRIANGLES,
            GLenum usage=GL_STATIC_DRAW);
        VertexArray(const Attributes& attributes,
            const std::vector<GLfloat>& vertices,
            GLenum mode=GL_TRIANGLES,
            GLenum usage=GL_STATIC_DRAW);
        VertexArray(const VertexArray& other);
        VertexArray& operator=(const VertexArray& other);
        virtual ~VertexArray() noexcept;

        std::shared_ptr<VBO> vbo(size_t index=0) const;
        std::shared_ptr<EBO> ebo() const;

        const std::vector<std::shared_ptr<VBO>>& vbos();

        virtual VertexArray* bind() override;
        virtual VertexArray* unbind() override;

        void draw() const;
        void drawInstanced(size_t instanceCount) const;

        std::shared_ptr<VBO> createVbo(GLenum usage,
            const lix::Attributes& attributes,
            void* data,
            GLuint byteLength,
            int attribDivisor=0);
        std::shared_ptr<VBO> createVbo(GLenum usage,
            const lix::Attributes& attributes,
            const std::vector<GLfloat>& vertices,
            int attribDivisor=0);
        std::shared_ptr<EBO> createEbo(GLenum usage, const std::vector<GLuint>& indices);
        std::shared_ptr<EBO> createEbo(GLenum usage, const std::vector<GLushort>& indices);

    private:
        const GLenum _mode{GL_TRIANGLES};
        std::vector<std::shared_ptr<VBO>> _vbos;
        std::shared_ptr<EBO> _ebo{nullptr};
    };

    using VAO = lix::VertexArray;
}