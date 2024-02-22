#pragma once

#include <vector>
#include "shape.h"

namespace impact
{
    void quickHull(const std::vector<glm::vec3>& s, std::vector<glm::vec3>& ch);

    bool gjk(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection);

    bool epa(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        glm::vec3& collisionVector, float& penetration);
}