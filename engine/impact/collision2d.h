#pragma once

#include "halfedge.h"
#include "shape.h"
#include <vector>

namespace lix {
bool gjk2d(Shape &a, Shape &b, std::vector<glm::vec3> &simplex,
           const glm::vec3 &initialDirection);

bool epa2d(Shape &a, Shape &b, std::vector<glm::vec3> &simplex,
           glm::vec3 &collisionVector, float &penetration);
} // namespace lix