#pragma once

#include <string>
#include <vector>
#include <list>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace gltf
{
    struct Texture
    {
        std::string name;
        int magFilter;
        int minFilter;
        int width;
        int height;
        int channels;
        const unsigned char* data;
        size_t data_size;
    };

    struct Material
    {   
        std::string name;
        glm::vec4 baseColor;
        float metallic;
        float roughness;
        const Texture* baseColorTexture;
    };

    struct Buffer
    {
        size_t index;
        enum Type { SCALAR, VEC2, VEC3, VEC4, MAT4 } type;
        int target;
        int componentType;
        unsigned char* data;
        size_t data_size;
    };

    struct Primitive
    {
        const Material* material;
        const Buffer** attributes;
        size_t attributes_size;
        const Buffer* indices;
    };

    struct Mesh
    {
        std::string name;
        const Primitive* primitives;
        size_t primitives_size;
    };

    struct Node
    {
        std::string name;
        const Mesh* mesh;
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        gltf::Node* parent;
        const gltf::Node** children;
        size_t children_size;
    };

    struct Scene
    {
        std::string name;
        const Node** nodes;
        size_t nodes_size;
    };

    struct Sampler
    {
        enum Interpolation{STEP, LINEAR} interpolation;
        const Buffer* input;
        const Buffer* output;
    };

    struct Channel
    {
        Sampler sampler;
        const Node* targetNode;
        std::string targetPath;
    };

    struct Animation
    {
        std::string name;
        const Channel* channels;
        size_t channels_size;
    };

    struct Skin
    {
        std::string name;
        const gltf::Buffer* inverseBindMatrices;
        const gltf::Node** joints;
        size_t joints_size;
    };
}