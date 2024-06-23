#pragma once

#include <vector>
#include "shape.h"
#include "halfedge.h"

namespace lix
{
    struct Collision
    {
        glm::vec3 contactPoint;
        glm::vec3 collisionNormal;
        float penetrationDepth;
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
    };
    
    bool gjk(Shape& a, Shape& b, std::vector<lix::Vertex>& simplex,
        const glm::vec3& initialDirection, lix::Collision* collision);

    bool epa(Shape& a, Shape& b, const std::vector<lix::Vertex>& simplex, lix::Collision* collision);
}