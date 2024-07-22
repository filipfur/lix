#pragma once

#include <vector>
#include "shape.h"
#include "halfedge.h"
#include "convexhull.h"

namespace lix
{
    struct Collision
    {
        glm::vec3 contactPoint;
        glm::vec3 normal;
        float penetrationDepth;
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
    };

    enum class gjk_state {
        ERROR,
        INCREMENTING,
        COLLISION,
        NO_COLLISION
    };

    bool collides(Shape& a, Shape& b, lix::Collision* collision);
    
    bool gjk(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection, lix::Collision* collision);

    void gjk_init();

    lix::gjk_state gjk_increment(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
        const glm::vec3& initialDirection, lix::Collision* collision);

    glm::vec3 getSupportPointOfA(Shape& shape, const glm::vec3& minkowskiSP);
    glm::vec3 getSupportPointOfB(Shape& shape, const glm::vec3& minkowskiSP);

    bool epa(Shape& a, Shape& b, const std::vector<glm::vec3>& simplex, lix::Collision* collision);
    bool epa_increment(Shape& a, Shape& b, lix::ConvexHull& ch, lix::Collision* collision);
}