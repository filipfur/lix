#pragma once

#include <memory>

#include "gltftypes.h"
#include "glmesh.h"
#include "glnode.h"
#include "glskinanimation.h"

namespace gltf
{
    lix::MeshPtr loadMesh(const gltf::Mesh& gltfMesh);
    lix::NodePtr loadNode(const gltf::Node& gltfNode);
    std::shared_ptr<lix::Skin> loadSkin(lix::Node* armatureNode,
        const gltf::Skin& gltfSkin);
    std::shared_ptr<lix::SkinAnimation> loadAnimation(lix::Node* armatureNode,
        const gltf::Animation& gltfAnimation);
}