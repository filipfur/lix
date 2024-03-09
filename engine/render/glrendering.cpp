#include "glrendering.h"

#include "gluniformbuffer.h"

#include <iostream>
#include <set>

void lix::renderQuad()
{
    static lix::VertexArray quadVAO{lix::Attributes{lix::VEC3, lix::VEC3, lix::VEC2}, {
        -0.5f, -0.5f, 0.0f,   +0.0f, +0.0f, +1.0f,   0.0f, 0.0f,
        +0.5f, -0.5f, 0.0f,   +0.0f, +0.0f, +1.0f,   1.0f, 0.0f,
        +0.5f, +0.5f, 0.0f,   +0.0f, +0.0f, +1.0f,   1.0f, 1.0f,
        -0.5f, +0.5f, 0.0f,   +0.0f, +0.0f, +1.0f,   0.0f, 1.0f
    }, {
        0, 1, 2,
        2, 3, 0
    }, GL_TRIANGLES, GL_STATIC_DRAW};
    quadVAO.bind();
    quadVAO.draw();
}

void lix::renderScreen()
{
    static lix::VertexArray screenVAO{
        {
            lix::VEC2, lix::VEC2
        },
        {
            -1.0f, -1.0f, 0.0f, 0.0f,
            +1.0f, -1.0f, 1.0f, 0.0f,
            +1.0f, +1.0f, 1.0f, 1.0f,
            -1.0f, +1.0f, 0.0f, 1.0f},
        {
            0, 1, 2,
            3, 0, 2
        },
        GL_TRIANGLES,
        GL_STATIC_DRAW
    };
    screenVAO.bind();
    screenVAO.draw();
}

void lix::bindMaterial(lix::ShaderProgram& shaderProgram, const lix::Material& material)
{
    shaderProgram.setUniform("u_base_color", material.baseColor());
    auto diffuse = material.diffuseMap();
    if(diffuse)
    {
        diffuse->bind(GL_TEXTURE0);
    }
}

void lix::renderMesh(lix::ShaderProgram& shaderProgram, lix::Mesh& mesh, const glm::mat4& model)
{
    shaderProgram.setUniform("u_model", model);
    for(size_t i{0}; i < mesh.count(); ++i)
    {
        auto mat = mesh.material(i);
        if(mat)
        {
            bindMaterial(shaderProgram, *mat);
        }
        mesh.draw(i);
    }
}

void lix::renderNode(lix::ShaderProgram& shaderProgram, lix::Node& node, bool recursive, bool globalMatrices)
{
    if(node.mesh())
    {
        renderMesh(shaderProgram, *node.mesh(), globalMatrices ? node.globalMatrix() : node.model());
    }
    if(!recursive)
    {
        return;
    }
    for(const auto& child : node.children())
    {
        renderNode(shaderProgram, *child, recursive, globalMatrices);
    }
}

struct JointBlock {
    glm::mat4 jointMatrices[24];
};

void lix::renderSkinAnimationNode(lix::ShaderProgram& shaderProgram, lix::Node& node)
{
    static JointBlock jointBlock{};
    static lix::UniformBuffer jointUBO{
        sizeof(JointBlock), (void*)&jointBlock, "JointBlock", 1
    };

    static std::set<int> boundShaders;
    auto it = boundShaders.find(shaderProgram.id());
    if(it == boundShaders.end())
    {
        jointUBO.bind();
        jointUBO.uniformBlockBinding(&shaderProgram);
        jointUBO.bindBufferBase();
        jointUBO.unbind();
        boundShaders.emplace(shaderProgram.id());
    }
    
    const glm::mat4 globalWorldInverse = glm::inverse(node.parent() ? node.parent()->model() : node.model());
    std::shared_ptr<lix::Skin> skin = node.skin();
    //static glm::mat4 jointMatrices[24];
    assert(skin->joints().size() <= 24);
    for(size_t j{0}; j < skin->joints().size(); ++j)
    {
        glm::mat4 globMat = skin->joint(j)->globalMatrix();
        jointBlock.jointMatrices[j] = globalWorldInverse * globMat * skin->inverseBindMatrices().at(j);
        //shaderProgram.setUniform("u_jointMatrix[" + std::to_string(j) + "]", jointMatrices[j]);
    }
    jointUBO.bind();
    jointUBO.bufferData();
    lix::renderNode(shaderProgram, node, true, true);
}