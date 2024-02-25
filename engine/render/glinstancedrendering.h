#pragma once

#include "glrendering.h"

namespace lix
{
    class InstancedRendering
    {
    public:
        InstancedRendering(MeshPtr mesh, const std::list<Node*>& nodes);

        void render(ShaderProgram& shaderProgram);

        void refresh();

    private:
        MeshPtr _mesh;
        std::list<Node*> _nodes;
        std::shared_ptr<lix::VertexArrayBuffer> _instancesVBO;
        std::vector<GLfloat> _instancesData;
    };
}