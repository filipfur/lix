#include "primer.h"

float impact::sign(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
}

bool impact::pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    float d1;
    float d2;
    float d3;
    d1 = sign(p, a, b);
    d2 = sign(p, b, c);
    d3 = sign(p, c, a);
    bool hasNeg = (d1 < 0 || d2 < 0 || d3 < 0);
    bool hasPos = (d1 > 0 || d2 > 0 || d3 > 0);
    return !(hasNeg && hasPos);
}