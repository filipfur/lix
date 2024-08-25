#pragma once

#include "glrendering.h"
#include <vector>

namespace lix {
template <class T> struct ModelAllocator {
    static lix::Attributes attributes() { return {lix::Attribute::MAT4}; }

    static void allocate(const T &t, std::vector<GLfloat> &buffer,
                         size_t index) {
        glm::mat4 m = glm::translate(glm::mat4{1.0f}, t);
        std::copy(glm::value_ptr(m), glm::value_ptr(m) + 16,
                  buffer.begin() + index * 16);
    }

    static void allocate(T &t, std::vector<GLfloat> &buffer, size_t index) {
        const glm::mat4 &m = t->modelMatrix();
        std::copy(glm::value_ptr(m), glm::value_ptr(m) + 16,
                  buffer.begin() + index * 16);
    }
};

template <typename Container,
          typename Allocator =
              lix::ModelAllocator<typename Container::value_type>>
class InstancedRendering {
  public:
    InstancedRendering(MeshPtr mesh, Container &instances,
                       GLenum usage = GL_STATIC_DRAW)
        : _mesh{mesh}, _instances{instances} {
        auto vao = mesh->vertexArray();
        vao->bind();

        for (const auto &vbo : vao->vbos()) {
            assert(vbo->attribDivisor() ==
                   0); // Assert that we arent already inst
        }

        _instancesVBO =
            vao->createVbo(usage, Allocator::attributes(), nullptr, 0, 1);

        allocateInstanceData();
    }

    void render(ShaderProgram &shaderProgram, size_t maxCount = SIZE_MAX) {
        auto vao = _mesh->vertexArray();
        std::shared_ptr<lix::Material> mat = _mesh->material(0);
        if (mat) {
            lix::bindMaterial(shaderProgram, *mat);
        }
        allocateInstanceData();
        vao->bind();
        vao->drawInstanced(std::min(maxCount, _instances.size()));
    }

  private:
    void allocateInstanceData() {
        size_t n = _instances.size();
        _buffer.resize(n * 16);
        auto it = _instances.begin();
        for (size_t i{0}; i < n; ++i) {
            Allocator::allocate(*it, _buffer, i);
            ++it;
        }
        _instancesVBO->bind();
        _instancesVBO->bufferData(_buffer);
    }

    MeshPtr _mesh;
    Container &_instances;
    std::shared_ptr<lix::VertexArrayBuffer> _instancesVBO;
    std::vector<GLfloat> _buffer;
};
} // namespace lix

template <>
inline void lix::ModelAllocator<lix::TRS>::allocate(
    lix::TRS &trs, std::vector<GLfloat> &buffer, size_t index) {
    const glm::mat4 &m = trs.modelMatrix();
    std::copy(glm::value_ptr(m), glm::value_ptr(m) + 16,
              buffer.begin() + index * 16);
}