#pragma once

#include "glm/glm.hpp"
#include <vector>

namespace lix {
void quickHull(const std::vector<glm::vec3> &s, std::vector<glm::vec3> &ch);

void quick_hull(const std::vector<glm::vec3> &points,
                std::vector<glm::vec3> &vertices,
                std::vector<unsigned int> &indices);
} // namespace lix