#pragma once

#include <unordered_map>

#include "gltftypes.h"
#include "glmesh.h"
#include "glnode.h"
#include "glskinanimation.h"

namespace gltf
{
    lix::MeshPtr loadMesh(const gltf::Mesh& gltfMesh)
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
        
        for(const auto& primitive : gltfMesh.primitives)
        {
            std::shared_ptr<lix::Material> material = nullptr;
            gltf::Material* mat = primitive.material;
            if(mat)
            {
                material = std::make_shared<lix::Material>(mat->baseColor, mat->metallic, mat->roughness);
                gltf::Texture* tex = mat->baseColorTexture;
                if(tex)
                {
                    static std::unordered_map<gltf::Texture*, std::shared_ptr<lix::Texture>> loadedTextures;
                    auto it = loadedTextures.find(tex);
                    if(it != loadedTextures.end())
                    {
                        //std::cout << "Already loaded: " << tex->name << std::endl;
                        material->setDiffuseMap(it->second);
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
                            tex->data.data(), tex->width, tex->height,
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
            for(gltf::Buffer* attrib : primitive.attributes)
            {
                size_t numBytes = attrib->data.size();
                GLfloat* fp = (GLfloat*)attrib->data.data();
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
                        attr = lix::Attribute::UVEC4; // TODO: Oversight
                        attr = lix::Attribute::VEC4;
                        attrib->componentType = GL_FLOAT;

                        std::vector<GLfloat> floatVec; // TODO: Quickfix, problem with glVertexAttribIPointer ?
                        floatVec.reserve(attrib->data.size());
                        for(unsigned char b : attrib->data)
                        {
                            floatVec.push_back(static_cast<float>(b));
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
                        std::vector<GLfloat>(fp, fp + (numBytes / sizeof(GLfloat))));
                    break;
                    case GL_UNSIGNED_BYTE:
                    prim.vao->createVbo(GL_STATIC_DRAW, {attr},
                        attrib->data.size(), componentSize, attrib->data.data(),
                        0,
                        attrib->componentType);
                    break;
                    default:
                    throw std::runtime_error("unkwown component type during gltf mesh loading (attributes)");
                    break;
                }
            }
            const auto& attrib = primitive.indices;
            size_t numBytes = attrib->data.size();
            GLushort* usp = (GLushort*)attrib->data.data();
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

    lix::NodePtr loadNode(const gltf::Node& gltfNode)
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

        for(gltf::Node* child : gltfNode.children)
        {
            node->appendChild(loadNode(*child));
        }

        return node;
    }

    std::shared_ptr<lix::Skin> loadSkin(lix::Node* armatureNode, const gltf::Skin& gltfSkin)
    {
        auto skin = std::make_shared<lix::Skin>();
        for(const auto& joint : gltfSkin.joints)
        {
            skin->joints().push_back(armatureNode->find(joint->name));
        }
        GLfloat* fp = (GLfloat*)gltfSkin.inverseBindMatrices->data.data();
        for(size_t j{0}; j < gltfSkin.joints.size(); ++j)
        {
            skin->inverseBindMatrices().push_back(glm::make_mat4(fp + j * 16));
        }
        armatureNode->setSkin(skin);
        return skin;
    }

    std::shared_ptr<lix::SkinAnimation> loadAnimation(lix::Node* armatureNode, const gltf::Animation& gltfAnimation)
    {
        auto anim = std::make_shared<lix::SkinAnimation>(gltfAnimation.name);

        float maxTime{0.0f};
        float minTime{FLT_MAX};

        for(const auto& channel : gltfAnimation.channels)
        {
            GLfloat* fp_times = (GLfloat*)channel.sampler.input->data.data();
            int numKeyFrames = channel.sampler.input->data.size() / 4;
            GLfloat* fp = (GLfloat*)channel.sampler.output->data.data();
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
            for(int i{0}; i < numKeyFrames; ++i)
            {
                float t = fp_times[i];
                int n;
                glm::vec3 v;
                glm::quat q;
                switch(ch.type())
                {
                    case lix::SkinAnimation::Channel::TRANSLATION:
                        n = 3 * i;
                        v = glm::vec3{fp[0 + n], fp[1 + n], fp[2 + n]};
                        //ch.node()->setTranslation(v);
                        ch.translations().emplace(t, v);
                        break;
                    case lix::SkinAnimation::Channel::ROTATION:
                        n = 4 * i;
                        q = glm::quat{fp[3 + n], fp[0 + n], fp[1 + n], fp[2 + n]};
                        //ch.node()->setRotation(q);
                        ch.rotations().emplace(t, q);
                        break;
                    case lix::SkinAnimation::Channel::SCALE:
                        n = 3 * i;
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
}