#include "primer.h"

#include "glgeometry.h"

#include <glm/gtc/quaternion.hpp>
#include <algorithm>

float lix::sign(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
}

bool lix::pointInTriangle2d(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
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

bool lix::pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    const glm::vec3 A = a - p;
    const glm::vec3 B = b - p;
    const glm::vec3 C = c - p;
    const glm::vec3 u = glm::cross(A, B);
    const glm::vec3 v = glm::cross(B, C);
    const glm::vec3 w = glm::cross(C, A);

    float d1 = glm::dot(u, v);
    float d2 = glm::dot(u, w);
    if(d1 < 0)
    {
        //printf("d1=%.2f d2=%.2f\n", d1, d2);
        return false;
    }
    if(d2 < 0)
    {
        //printf("d1=%.2f d2=%.2f\n", d1, d2);
        return false;
    }
    return true;
}

glm::vec3 lix::barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 v0 = b - a;
    glm::vec3 v1 = c - a;
    glm::vec3 v2 = p - a;
    
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    
    glm::vec3 barycentric;
    barycentric[0] = (d11 * d20 - d01 * d21) / denom;
    barycentric[1] = (d00 * d21 - d01 * d20) / denom;
    barycentric[2] = 1.0f - barycentric[0] - barycentric[1];
    return barycentric;
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

std::vector<glm::vec3> lix::uniqueVertices(const std::vector<glm::vec3>& s)
{
    std::vector<glm::vec3> u;
    for(const auto& p0 : s)
    {
        if(!containsVertex(u, p0))
        {
            u.push_back(p0);
        }
    }
    return u;
}

std::pair<glm::vec3, glm::vec3> lix::extremePoints(const std::vector<glm::vec3>& s)
{
    glm::vec3 min{FLT_MAX};
    glm::vec3 max{-FLT_MAX};

    for(const auto& p0 : s)
    {
        max.x = std::max(max.x, p0.x);
        max.y = std::max(max.y, p0.y);
        max.z = std::max(max.z, p0.z);

        min.x = std::min(min.x, p0.x);
        min.y = std::min(min.y, p0.y);
        min.z = std::min(min.z, p0.z);
    }
    return { min, max };
}

std::vector<glm::vec3> lix::minimumBoundingBox(const glm::vec3& min, const glm::vec3& max)
{
    return lix::cube_corner_points(min, max);
}

int lix::indexAlongDirection(const std::vector<glm::vec3>& s, const glm::vec3& D)
{
    int index{-1};
    float maxValue{-FLT_MAX}; //glm::dot(s[0], D)};

    for(size_t i{0}; i < s.size(); ++i)
    {
        float value = glm::dot(s[i], D);
        if(value > maxValue)
        {
            index = static_cast<int>(i);
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
                edge.first = static_cast<unsigned int>(aa);
                edge.second = static_cast<unsigned int>(ab);
                return true;
            }
            else if(aa == bb && ab == ba)
            {
                edge.first = static_cast<unsigned int>(aa);
                edge.second = static_cast<unsigned int>(ab);
                return true;
            }
        }
    }
    return false;
}

glm::quat lix::directionToQuat(const glm::vec3 &direction)
{
    glm::vec3 n = glm::normalize(direction);
    glm::vec3 UP{0.0f, 1.0f, 0.0f};
    float angle;
    glm::vec3 axis;

    if (n.y >= 1.0f)
    {
        angle = 0.0f;
        axis = glm::vec3{1.0f, 0.0f, 0.0f};
    }
    else if (n.y <= -1.0f)
    {
        angle = glm::pi<float>();
        axis = glm::vec3{1.0f, 0.0f, 0.0f};
    }
    else
    {
        axis = glm::normalize(glm::cross(UP, n));
        angle = acosf(glm::dot(UP, n));
    }

    return glm::angleAxis(angle, axis);
}

glm::vec3 lix::quatToDirection(const glm::quat &quat)
{
    glm::vec3 UP{0.0f, 1.0f, 0.0f};
    return quat * UP;
}