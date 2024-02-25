#pragma once

#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include "glvertexarray.h"
#include "gltrs.h"
#include "glshaderprogram.h"

namespace lix
{
    class PolygonRendering
    {
    public:
        PolygonRendering(const std::vector<glm::vec3>& points, const std::vector<std::shared_ptr<lix::TRS>>& instances);

        void setPoints(const std::vector<glm::vec3>& points);

        void addInstance(std::shared_ptr<lix::TRS> trs);

        void render(std::shared_ptr<lix::ShaderProgram> shaderProgram);

    private:
        std::vector<glm::vec3> _points;
        std::shared_ptr<lix::VAO> _vao;
        std::vector<std::shared_ptr<lix::TRS>> _instances;
    };
}