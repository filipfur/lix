#pragma once

#include <memory>

#include "gltftypes.h"
#include "glmesh.h"
#include "glnode.h"
#include "glskinanimation.h"
#include "polygon.h"

namespace gltf
{
    lix::MeshPtr loadMesh(const gltf::Mesh& gltfMesh);
    lix::NodePtr loadNode(const gltf::Node& gltfNode);
    std::shared_ptr<lix::Skin> loadSkin(lix::Node* armatureNode,
        const gltf::Skin& gltfSkin);
    std::shared_ptr<lix::SkinAnimation> loadAnimation(lix::Node* armatureNode,
        const gltf::Animation& gltfAnimation);

    std::vector<glm::vec3> loadVertexPositions(const gltf::Mesh& mesh);

    enum VAttrib
    {
        A_POSITION=1,
        A_NORMAL=2,
        A_TEXCOORD=4
    };

    template <typename T, typename U>
    void loadAttributes(const gltf::Mesh& mesh, int primitiveIdx, uint32_t attribs, std::vector<T>& vertices, std::vector<U>& indices)
    {
        //assert(primitive.indices->componentType == GL_UNSIGNED_SHORT);

        const auto& primitive = mesh.primitives[primitiveIdx];

        {
            size_t numBytes = primitive.indices->data_size;
            U* buf = (U*)primitive.indices->data;
            indices = std::vector<U>(buf, buf + (numBytes / sizeof(U)));
        }

        int j{0};
        int n{0};
        for(int i{1}; i < 64; i *= 2)
        {
            if((i & attribs) != 0)
            {
                auto attrib = primitive.attributes[j];
                size_t numBytes = attrib->data_size;
                T* buf = (T*)attrib->data;
                printf("numbytes=%zu sizeof(T)=%zu\n", numBytes, sizeof(T));
                vertices = std::vector<T>(buf, buf + (numBytes / sizeof(T)));
                ++n;
                if(n > 1)
                {
                    perror("only one attrib is supported for now...\n");
                    exit(1);
                }
            }
            ++j;
        }
    }

    std::shared_ptr<lix::Polygon> loadMeshCollider(const gltf::Mesh& gltfMesh);
}