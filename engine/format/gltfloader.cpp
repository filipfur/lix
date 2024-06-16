#include "gltfloader.h"

#include <stdexcept>
#include <unordered_map>
#include "glm/gtc/type_ptr.hpp"

template <typename T>
std::vector<T> toVector(const gltf::Buffer& buffer)
{
    size_t numBytes = buffer.data_size;
    const T* buf = (const T*)buffer.data;
    return std::vector<T>(buf, buf + (numBytes / sizeof(T)));
}

lix::MeshPtr gltf::loadMesh(const gltf::Mesh& gltfMesh)
{
    static std::unordered_map<const gltf::Mesh*, lix::MeshPtr> loadedMeshes;
    auto it = loadedMeshes.find(&gltfMesh);
    if(it != loadedMeshes.end())
    {
        //std::cout << "Already loaded: " << gltfMesh.name << std::endl;
        return it->second;
    }
    lix::MeshPtr mesh = std::make_shared<lix::Mesh>();
    loadedMeshes.emplace(&gltfMesh, mesh);
    
    for(size_t i{0}; i < gltfMesh.primitives_size; ++i)//const auto& primitive : gltfMesh.primitives)
    {
        const gltf::Primitive& primitive = gltfMesh.primitives[i];
        std::shared_ptr<lix::Material> material = nullptr;
        const gltf::Material* mat = primitive.material;
        if(mat)
        {
            material = std::make_shared<lix::Material>(mat->baseColor, mat->metallic, mat->roughness);
            const gltf::Texture* tex = mat->baseColorTexture;
            if(tex)
            {
                static std::unordered_map<const gltf::Texture*, std::shared_ptr<lix::Texture>> loadedTextures;
                auto loadedTexIt = loadedTextures.find(tex);
                if(loadedTexIt != loadedTextures.end())
                {
                    //std::cout << "Already loaded: " << tex->name << std::endl;
                    material->setDiffuseMap(loadedTexIt->second);
                }
                else
                {
                    GLenum format;
                    GLenum internalFormat;
                    switch(tex->channels)
                    {
                        case 1:
                            format = GL_RED;
                            internalFormat = GL_RED;
                            break;
                        case 3:
                            format = GL_RGB;
                            internalFormat = GL_SRGB;
                            internalFormat = GL_RGB;
                            break;
                        case 4:
                            format = GL_RGBA;
                            internalFormat = GL_SRGB_ALPHA;
                            internalFormat = GL_RGBA;
                            break;
                        default:
                            throw std::runtime_error("don't know how to convert channels to format");
                            break;
                    }
                    auto diffuseMap = std::make_shared<lix::Texture>(
                        tex->data, tex->width, tex->height,
                        GL_UNSIGNED_BYTE, internalFormat, format
                    );
                    diffuseMap->bind();
                    diffuseMap->setFilter(tex->magFilter); // tex->minFilter
                    diffuseMap->setWrap(GL_REPEAT);
                    material->setDiffuseMap(diffuseMap);
                    loadedTextures.emplace(tex, diffuseMap);
                }
            }
            else {
                static lix::TexturePtr basicTexture = lix::Texture::Basic();
                material->setDiffuseMap(basicTexture);
            }
        }
        const auto& prim = mesh->createPrimitive(GL_TRIANGLES, material);
        prim.vao->bind();
        for(size_t j{0}; j < primitive.attributes_size; ++j)// const gltf::Buffer* attrib : primitive.attributes)
        {
            const gltf::Buffer* attrib = primitive.attributes[j];
            lix::Attribute attr;
            assert(attrib->target == GL_ARRAY_BUFFER);
            GLuint componentSize{1};
            switch(attrib->type)
            {
                case gltf::Buffer::VEC2:
                attr = lix::Attribute::VEC2;
                componentSize = sizeof(glm::vec2);
                break;
                case gltf::Buffer::VEC3:
                attr = lix::Attribute::VEC3;
                componentSize = sizeof(glm::vec3);
                break;
                case gltf::Buffer::VEC4:
                if(attrib->componentType == GL_UNSIGNED_BYTE)
                {
                    //attr = lix::Attribute::UVEC4; // TODO: Oversight
                    attr = lix::Attribute::VEC4;
                    //attrib->componentType = GL_FLOAT;

                    std::vector<GLfloat> floatVec; // TODO: Quickfix, problem with glVertexAttribIPointer ?
                    floatVec.reserve(attrib->data_size);
                    for(size_t k{0}; k < attrib->data_size; ++k)
                    {
                        floatVec.push_back(static_cast<float>(attrib->data[k]));
                    }
                    prim.vao->createVbo(GL_STATIC_DRAW, {attr}, floatVec);
                    continue;
                }
                else
                {
                    attr = lix::Attribute::VEC4;
                }
                componentSize = sizeof(glm::vec4);
                break;
                case gltf::Buffer::MAT4:
                attr = lix::Attribute::MAT4;
                componentSize = sizeof(glm::mat4);
                break;
                default:
                throw std::runtime_error("failed during gltf attribute conversion");
                break;
            }
            switch(attrib->componentType)
            {
                case GL_FLOAT:
                prim.vao->createVbo(GL_STATIC_DRAW, {attr},
                    toVector<GLfloat>(*attrib));
                break;
                case GL_UNSIGNED_BYTE:
                prim.vao->createVbo(GL_STATIC_DRAW, {attr},
                    static_cast<GLuint>(attrib->data_size), componentSize, attrib->data,
                    0,
                    attrib->componentType);
                break;
                default:
                throw std::runtime_error("unkwown component type during gltf mesh loading (attributes)");
                break;
            }
        }
        const gltf::Buffer* attrib = primitive.indices;
        size_t numBytes = attrib->data_size;
        GLushort* usp = (GLushort*)attrib->data;
        assert(attrib->target == GL_ELEMENT_ARRAY_BUFFER);
        switch(attrib->componentType)
        {
        case GL_UNSIGNED_SHORT:
            prim.vao->createEbo(GL_STATIC_DRAW, std::vector<GLushort>(usp, usp + (numBytes / sizeof(GLushort))));
            break;
        default:
            throw std::runtime_error("unkwown component type during gltf mesh loading (indices)");
            break;
        }
    }
    return mesh;
}

lix::NodePtr gltf::loadNode(const gltf::Node& gltfNode)
{
    lix::NodePtr node = std::make_shared<lix::Node>();
    node->setName(gltfNode.name);
    node->setTranslation(gltfNode.translation);
    node->setRotation(gltfNode.rotation);
    node->setScale(gltfNode.scale);
    if(gltfNode.mesh)
    {
        node->setMesh(loadMesh(*gltfNode.mesh));
    }

    for(size_t i{0}; i < gltfNode.children_size; ++i)//const gltf::Node* child : gltfNode.children)
    {
        node->appendChild(loadNode(*gltfNode.children[i]));
    }

    return node;
}

std::shared_ptr<lix::Skin> gltf::loadSkin(lix::Node* armatureNode, const gltf::Skin& gltfSkin)
{
    auto skin = std::make_shared<lix::Skin>();
    for(size_t i{0}; i < gltfSkin.joints_size; ++i)//const auto& joint : gltfSkin.joints)
    {
        const gltf::Node& joint = *gltfSkin.joints[i];
        skin->joints().push_back(armatureNode->find(joint.name));
    }
    const GLfloat* fp = (const GLfloat*)gltfSkin.inverseBindMatrices;
    for(size_t j{0}; j < gltfSkin.joints_size; ++j)
    {
        skin->inverseBindMatrices().push_back(glm::make_mat4(fp + j * 16));
    }
    armatureNode->setSkin(skin);
    return skin;
}

std::shared_ptr<lix::SkinAnimation> gltf::loadAnimation(lix::Node* armatureNode, const gltf::Animation& gltfAnimation)
{
    auto anim = std::make_shared<lix::SkinAnimation>(gltfAnimation.name);

    float maxTime{0.0f};
    float minTime{FLT_MAX};

    for(size_t i{0}; i < gltfAnimation.channels_size; ++i) //const auto& channel : gltfAnimation.channels)
    {
        const gltf::Channel& channel = gltfAnimation.channels[i];
        GLfloat* fp_times = (GLfloat*)channel.sampler.input->data;
        int numKeyFrames{static_cast<int>(channel.sampler.input->data_size) / 4};
        GLfloat* fp = (GLfloat*)channel.sampler.output->data;
        lix::SkinAnimation::Channel& ch = anim->channels().emplace_back();

        minTime = std::min(minTime, fp_times[0]);
        maxTime = std::max(maxTime, fp_times[numKeyFrames - 1]);
        if(channel.targetPath == "translation")
        {
            ch.setType(lix::SkinAnimation::Channel::TRANSLATION);
        }
        else if(channel.targetPath == "rotation")
        {
            ch.setType(lix::SkinAnimation::Channel::ROTATION);
        }
        else if(channel.targetPath == "scale")
        {
            ch.setType(lix::SkinAnimation::Channel::SCALE);
        }

        ch.setNode(armatureNode->find(channel.targetNode->name));
        for(int j{0}; j < numKeyFrames; ++j)
        {
            float t = fp_times[j];
            int n;
            glm::vec3 v;
            glm::quat q;
            switch(ch.type())
            {
                case lix::SkinAnimation::Channel::TRANSLATION:
                    n = 3 * j;
                    v = glm::vec3{fp[0 + n], fp[1 + n], fp[2 + n]};
                    //ch.node()->setTranslation(v);
                    ch.translations().emplace(t, v);
                    break;
                case lix::SkinAnimation::Channel::ROTATION:
                    n = 4 * j;
                    q = glm::quat{fp[3 + n], fp[0 + n], fp[1 + n], fp[2 + n]};
                    //ch.node()->setRotation(q);
                    ch.rotations().emplace(t, q);
                    break;
                case lix::SkinAnimation::Channel::SCALE:
                    n = 3 * j;
                    v = glm::vec3{fp[0 + n], fp[1 + n], fp[2 + n]};
                    //ch.node()->setScale(v);
                    ch.scales().emplace(t, v);
                    break;
            }
        }
    }
    anim->setTime(minTime);
    anim->setStart(minTime);
    anim->setEnd(maxTime + 1.0f / 27.0f);
    if(armatureNode->skin())
    {
        armatureNode->skin()->addAnimation(anim->name(), anim);
    }
    return anim;
}

std::vector<glm::vec3> gltf::loadVertexPositions(const gltf::Mesh& mesh)
{
    std::vector<glm::vec3> vertexPositions;

    for(size_t i{0}; i < mesh.primitives_size; ++i)//(const auto& primitive : mesh.primitives)
    {
        const Primitive& primitive = mesh.primitives[i];
        assert(primitive.indices->componentType == GL_UNSIGNED_SHORT);
        std::vector<GLushort> indices = toVector<GLushort>(*primitive.indices);
        vertexPositions.resize(indices.size());
        std::vector<GLfloat> positions = toVector<GLfloat>(*primitive.attributes[0]);

        for(size_t j{0}; j < indices.size(); ++j)
        {
            auto idx = indices[j];
            vertexPositions[j].x = positions[idx * 3 + 0];
            vertexPositions[j].y = positions[idx * 3 + 1];
            vertexPositions[j].z = positions[idx * 3 + 2];
        }
        //break; // first primitive only!
    }
    return vertexPositions;
}