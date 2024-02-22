#pragma once

#include <list>
#include <memory>

#include "glmesh.h"
#include "glnode.h"
#include "glshaderprogram.h"

namespace lix
{
    void renderScreen();
    void renderQuad();
    void bindMaterial(lix::ShaderProgram& shaderProgram, const lix::Material& material);
    void renderMesh(lix::ShaderProgram& shaderProgram, lix::Mesh& mesh, const glm::mat4& model);
    void renderNode(lix::ShaderProgram& shaderProgram, lix::Node& node, bool recursive=true, bool globalMatrices=true);
    void renderSkinAnimationNode(lix::ShaderProgram& shaderProgram, lix::Node& node);
}