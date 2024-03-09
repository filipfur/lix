#pragma once

#include <vector>
#include "shape.h"

namespace impact
{
    bool gjk(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection);

    bool epa(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        glm::vec3& collisionVector, float& penetration);
}