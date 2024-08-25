#pragma once

#include "glm/glm.hpp"
#include "glshaderprogram.h"
#include "gltrs.h"
#include "glvertexarray.h"
#include <memory>
#include <vector>

namespace lix {
class PolygonRendering {
  public:
    PolygonRendering(const std::vector<glm::vec3> &points,
                     const std::vector<lix::TRS *> &instances);

    void setPoints(const std::vector<glm::vec3> &points);

    void render(std::shared_ptr<lix::ShaderProgram> shaderProgram);

  private:
    std::vector<glm::vec3> _points;
    std::shared_ptr<lix::VAO> _vao;
    const std::vector<lix::TRS *> &_instances;
};
} // namespace lix