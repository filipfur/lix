#pragma once

#include <vector>
#include "shape.h"

namespace lix
{
    struct Collision
    {
        glm::vec3 collisionNormal;
        float penetrationDepth;
    };

    bool gjk2d(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection);
    
    bool gjk(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection, lix::Collision* collision);

    bool epa(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        glm::vec3& collisionVector, float& penetration);
}