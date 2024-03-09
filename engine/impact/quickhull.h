#pragma once

#include <vector>
#include "glm/glm.hpp"

namespace impact
{
    void quickHull(const std::vector<glm::vec3>& s, std::vector<glm::vec3>& ch);
}