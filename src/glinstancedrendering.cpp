#include "glinstancedrendering.h"

lix::InstancedRendering::InstancedRendering(lix::MeshPtr mesh, const std::list<Node*>& nodes) : _mesh{mesh}, _nodes{nodes}
{
    auto vao = mesh->vertexArray();
    vao->bind();
    _instancesData.resize(nodes.size() * 16); // 16 floats ie mat4 per node
    auto nodeIt = nodes.begin();
    for(size_t i{0}; i < nodes.size(); ++i)
    {
        const glm::mat4& m = (*nodeIt)->globalMatrix();
        std::copy(glm::value_ptr(m), glm::value_ptr(m) + 16, _instancesData.begin() + i * 16);
        ++nodeIt;
    }

    for(const auto& vbo : vao->vbos())
    {
        assert(vbo->attribDivisor() == 0); // Assert that we arent already inst
    }

    _instancesVBO = vao->createVbo(GL_DYNAMIC_DRAW, {lix::Attribute::MAT4},
        _instancesData, 1);
}

void lix::InstancedRendering::refresh()
{
    _instancesData.resize(_nodes.size() * 16);
    auto nodeIt = _nodes.begin();
    for(size_t i{0}; i < _nodes.size(); ++i)
    {
        const glm::mat4& m = (*nodeIt)->globalMatrix();
        std::copy(glm::value_ptr(m), glm::value_ptr(m) + 16, _instancesData.begin() + i * 16);
        ++nodeIt;
    }
    _instancesVBO->bind();
    _instancesVBO->bufferData(_instancesData);
}

void lix::InstancedRendering::render(ShaderProgram& shaderProgram)
{
    auto vao = _mesh->vertexArray();
    std::shared_ptr<lix::Material> mat = _mesh->material(0);
    if(mat)
    {
        lix::bindMaterial(shaderProgram, *mat);
    }
    vao->bind();
    vao->drawInstanced(_nodes.size());
}