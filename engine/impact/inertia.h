#pragma once

#include <glm/glm.hpp>

namespace lix
{
    glm::mat3 cubeInertiaTensor(float mass, float sideLength);
} // namespace lix
