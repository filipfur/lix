#pragma once

#include "common.h"
#include "gltftypes.h"

inline std::string textureName(const gltf::Texture& texture)
{
    return common::variableName(texture.name) + "_tex";
}

inline std::string materialName(const gltf::Material& material)
{
    return common::variableName(material.name) + "_mat";
}

inline std::string meshName(const gltf::Mesh& mesh)
{
    return common::variableName(mesh.name) + "_mesh";
}

inline std::string nodeName(const gltf::Node& node)
{
    return common::variableName(node.name) + "_node";
}

inline std::string sceneName(const gltf::Scene& scene)
{
    return common::variableName(scene.name) + "_scene";
}

inline std::string animationName(const gltf::Animation& animation)
{
    return common::variableName(animation.name) + "_anim";
}

inline std::string skinName(const gltf::Skin& skin)
{
    return common::variableName(skin.name) + "_skin";
}

void exportBuffer(const gltf::Buffer& buffer, size_t bufferIndex, size_t offset, std::ofstream& ofs, const std::string& tab);

void exportVec3(std::ofstream& ofs, const glm::vec3& v);

void exportQuat(std::ofstream& ofs, const glm::quat& q);

void exportNode(std::ofstream& ofs, const std::string& scope,
    const gltf::Node& node);

void exportNodeChildren(std::ofstream& ofs, const std::string& scope,
    const gltf::Node& node);

void exportTexture(std::ofstream& ofs, const std::string& scope, const gltf::Texture& texture);

void exportMaterial(std::ofstream& ofs, const std::string& scope, const gltf::Material& material);

void exportAnimation(std::ofstream& ofs, const std::string& scope, const gltf::Animation& animation);

void exportMesh(std::ofstream& ofs, const std::string& scope, const gltf::Mesh& mesh);

void exportSkin(std::ofstream& ofs, const std::string& scope, const gltf::Skin& skin);