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
        std::vector<unsigned char> data;
    };

    struct Material
    {   
        std::string name;
        glm::vec4 baseColor;
        float metallic;
        float roughness;
        Texture* baseColorTexture;
    };

    struct Buffer
    {
        size_t index;
        enum Type { SCALAR, VEC2, VEC3, VEC4, MAT4 } type;
        int target;
        int componentType;
        std::vector<unsigned char> data;
    };

    struct Primitive
    {
        Material* material;
        std::vector<Buffer*> attributes;
        Buffer* indices;
    };

    struct Mesh
    {
        std::string name;
        std::vector<Primitive> primitives;
    };

    struct Node
    {
        std::string name;
        Mesh* mesh;
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        gltf::Node* parent;
        std::list<gltf::Node*> children;
    };

    struct Scene
    {
        std::string name;
        std::list<Node*> nodes;
    };

    struct Sampler
    {
        enum Interpolation{STEP, LINEAR} interpolation;
        Buffer* input;
        Buffer* output;
    };

    struct Channel
    {
        Sampler sampler;
        Node* targetNode;
        std::string targetPath;
    };

    struct Animation
    {
        std::string name;
        std::vector<Channel> channels;
    };

    struct Skin
    {
        std::string name;
        gltf::Buffer* inverseBindMatrices;
        std::vector<gltf::Node*> joints;
    };
}