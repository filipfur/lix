#pragma once

#include <vector>
#include "glrendering.h"

namespace lix
{
    class InstancedRendering
    {
    public:
        InstancedRendering(MeshPtr mesh, std::vector<lix::TRS>& instances);

        void render(ShaderProgram& shaderProgram, size_t maxCount=SIZE_MAX);

        void refresh(size_t maxCount=SIZE_MAX);

    private:
        void allocateInstanceData(size_t maxCount);

        MeshPtr _mesh;
        std::vector<lix::TRS>& _instances;
        std::shared_ptr<lix::VertexArrayBuffer> _instancesVBO;
        std::vector<GLfloat> _instancesData;
    };
}