#pragma once

#include "glrendering.h"

namespace lix
{
    class InstancedNodeRendering
    {
    public:
        InstancedNodeRendering(MeshPtr mesh, const std::list<lix::NodePtr>& nodes);

        void render(ShaderProgram& shaderProgram, size_t maxCount=SIZE_MAX);

        void refresh(size_t maxCount=SIZE_MAX);

    private:
        void allocateInstanceData(size_t maxCount);

        MeshPtr _mesh;
        const std::list<std::shared_ptr<Node>>& _nodes;
        std::shared_ptr<lix::VertexArrayBuffer> _instancesVBO;
        std::vector<GLfloat> _instancesData;
    };
}