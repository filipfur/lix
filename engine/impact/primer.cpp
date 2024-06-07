#include "primer.h"

#include <algorithm>

float lix::sign(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
}

bool lix::pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
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

bool lix::isSameVertex(const glm::vec3& a, const glm::vec3& b)
{
    glm::vec3 c = b - a;
    return glm::dot(c, c) < FLT_EPSILON;
}

/*
x/12=(x-420)/2+4
x=588

x=920
x/20=(x-707)/3-25
*/

bool lix::containsVertex(const std::vector<glm::vec3>& s, const glm::vec3& v)
{
    auto it = std::find_if(std::begin(s), std::end(s), [&v](const glm::vec3& p1) {
            return isSameVertex(v, p1);
    });
    return it != s.end();
}

void lix::uniqueVertices(const std::vector<glm::vec3>& s, std::vector<glm::vec3>& u)
{
    for(const auto& p0 : s)
    {
        if(!containsVertex(u, p0))
        {
            u.push_back(p0);
        }
    }
}

int lix::indexAlongDirection(const std::vector<glm::vec3>& s, const glm::vec3& D)
{
    int index{-1};
    float maxValue{0.0f};

    for(size_t i{0}; i < s.size(); ++i)
    {
        float value = glm::dot(s[i], D);
        if(value > maxValue)
        {
            index = i;
            maxValue = value;
        }
    }
    return index;
}

bool lix::isAdjacent(const unsigned int* a, const unsigned int* b, lix::Edge& edge)
{
    for(size_t i{0UL}; i < 3UL; ++i)
    {
        for(size_t j{0UL}; j < 3UL; ++i)
        {
            size_t aa = a[i];
            size_t ba = b[j];
            size_t ab = a[(i + 1) % 3];
            size_t bb = b[(j + 1) % 3];

            if(aa == ba && ab == bb)
            {
                edge.first = aa;
                edge.second = ab;
                return true;
            }
            else if(aa == bb && ab == ba)
            {
                edge.first = aa;
                edge.second = ab;
                return true;
            }
        }
    }
    return false;
}