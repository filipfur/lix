#pragma once
   
#include <vector>
#include "shape.h"
#include "halfedge.h"

namespace lix
{
    bool gjk2d(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection);

    bool epa2d(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        glm::vec3& collisionVector, float& penetration);
}