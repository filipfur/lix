#include "glinstancednoderendering.h"

lix::InstancedNodeRendering::InstancedNodeRendering(lix::MeshPtr mesh,
    const std::list<std::shared_ptr<Node>>& nodes)
    : _mesh{mesh}, _nodes{nodes}
{
    auto vao = mesh->vertexArray();
    vao->bind();
    allocateInstanceData(SIZE_MAX);

    for(const auto& vbo : vao->vbos())
    {
        assert(vbo->attribDivisor() == 0); // Assert that we arent already inst
    }

    _instancesVBO = vao->createVbo(GL_DYNAMIC_DRAW, {lix::Attribute::MAT4},
        _instancesData, 1);
}

void lix::InstancedNodeRendering::render(ShaderProgram& shaderProgram, size_t maxCount)
{
    auto vao = _mesh->vertexArray();
    std::shared_ptr<lix::Material> mat = _mesh->material(0);
    if(mat)
    {
        lix::bindMaterial(shaderProgram, *mat);
    }
    vao->bind();
    vao->drawInstanced(std::min(_nodes.size(), maxCount));
}

void lix::InstancedNodeRendering::refresh(size_t maxCount)
{
    allocateInstanceData(maxCount);
    _instancesVBO->bind();
    _instancesVBO->bufferData(_instancesData);
}

void lix::InstancedNodeRendering::allocateInstanceData(size_t maxCount)
{
    size_t size = std::min(_nodes.size(), maxCount);
    _instancesData.resize(size * 16);
    auto nodeIt = _nodes.begin();
    for(size_t i{0}; i < size; ++i)
    {
        const glm::mat4& m = (*nodeIt)->globalMatrix();
        std::copy(glm::value_ptr(m), glm::value_ptr(m) + 16, _instancesData.begin() + i * 16);
        ++nodeIt;
    }
}