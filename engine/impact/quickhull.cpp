#include "quickhull.h"

#include <algorithm>

#include "primer.h"

#include "glm/gtc/random.hpp"

float distanceFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    // TODO: Optimize (not needing actual distance)
    // TODO: Cache results since line isn't moving.
    return glm::abs((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y)) / glm::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
};

float signDistanceFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    // TODO: Optimize (not needing actual distance)
    // TODO: Cache results since line isn't moving.
    return ((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y)) / glm::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
};

float magnitudeFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    return glm::abs((b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y));
};

float signFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& p)
{
    return (b.x - a.x) * (a.y - p.y) - (a.x - p.x) * (b.y - a.y);
};

void findHull(const std::vector<glm::vec3>& s, glm::vec3 p,
    glm::vec3 q, std::vector<glm::vec3>& CH) {
    if(s.empty())
    {
        return;
    }
    float maxDistance{0};
    auto ret = s.end();
    for(auto it=s.begin(); it != s.end(); ++it)
    {
        float d = distanceFromLine(p, q, *it);
        if(d > maxDistance)
        {
            maxDistance = d;
            ret = it;
        }
    }
    CH.push_back(*ret);
    std::vector<glm::vec3> s1;
    std::vector<glm::vec3> s2;
    for(const auto& x : s)
    {
        if(lix::pointInTriangle2d(x, p, *ret, q))
        {
            //s0.insert(...);
        }
        else if(signFromLine(*ret, p, x) > 0)
        {
            s1.push_back(x);
        }
        else if(signFromLine(q, *ret, x) > 0)
        {
            s2.push_back(x);
        }
    }
    findHull(s1, *ret, p, CH);
    findHull(s2, q, *ret, CH);
}

void lix::quickHull(const std::vector<glm::vec3>& s, std::vector<glm::vec3>& ch)
{
    std::vector<glm::vec3> s1;
    std::vector<glm::vec3> s2;

    const glm::vec3& a = s.front();
    const glm::vec3& b = s.back();
    ch.push_back(a);

    for(size_t i{1}; i < s.size() - 1; ++i)
    {
        const glm::vec3& p = s.at(i);
        float d = signFromLine(a, b, p);
        if(d < 0)
        {
            s1.push_back(p);
        }
        else if(d > 0)
        {
            s2.push_back(p);
        }
        else
        {
            ;
        }
    }

    findHull(s1, a, b, ch);
    findHull(s2, b, a, ch);

    ch.push_back(b);

    glm::vec3 center{0.0f, 0.0f, 0.0f};
    float ratio = 1.0f / static_cast<float>(ch.size());
    for(const auto& p : ch)
    {
        center += p * ratio;
    }

    std::sort(ch.begin(), ch.end(), [&center](const glm::vec3& a, const glm::vec3& b) -> bool {
        return atan2(a.y - center.y, a.x - center.x) < atan2(b.y - center.y, b.x - center.x);
    });
}

glm::vec3 popAlongDirection(std::vector<glm::vec3>& s, const glm::vec3& D)
{
    auto i = lix::indexAlongDirection(s, D);
    glm::vec3 rval = s[i];
    s.erase(s.begin() + i);
    return rval;
}

//assumes s is centered around the origin
void lix::quick_hull(const std::vector<glm::vec3>& points, std::vector<glm::vec3>& vertices, std::vector<unsigned int>& faces)
{
    std::vector<glm::vec3> P{points.begin(), points.end()};

    //float BIG = 1.0e32f;
    glm::vec3 dir = glm::ballRand(1.0f);
    glm::vec3 A = popAlongDirection(P, dir);
    glm::vec3 B = popAlongDirection(P, -A);

    glm::vec3 AB = B - A;
    float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
    glm::vec3 r = A + AB * t;

    assert(glm::dot(r, r) > 0);

    glm::vec3 C = popAlongDirection(P, -r);

    glm::vec3 AC = C - A;
    glm::vec3 ABC = glm::cross(AB, AC);

    if(glm::dot(ABC, -A) > 0)
    {
        std::swap(B, C);
        std::swap(AC, AB);
        ABC = -ABC;
    }

    glm::vec3 D = popAlongDirection(P, -ABC);

    //const glm::vec3 AD = D - A;
    //const glm::vec3 ABD = glm::cross(AB, AD);

    vertices.insert(vertices.end(), {A, B, C, D});

    faces.insert(faces.end(), {
        0, 1, 2, // ABC
        0, 2, 3, // ACD
        0, 3, 1, // ADB
        1, 3, 2  // BDC
    });

    std::vector<glm::vec3> normals;

    for(size_t f{0UL}; f < faces.size(); f += 3UL)
    {
        const glm::vec3& U = vertices.at(faces.at(f));
        const glm::vec3& V = vertices.at(faces.at(f + 1));
        const glm::vec3& W = vertices.at(faces.at(f + 2));
        normals.push_back(glm::cross(V - U, W - U));
    }

    std::vector<unsigned int> Q = {0, 1, 2, 3};

    while(!Q.empty())
    {
        auto face = Q.front();
        Q.erase(Q.begin());

        int pIdx = -1;
        float pVal = 0.0f;
        for(size_t i{0UL}; i < P.size(); ++i)
        {
            float val = glm::dot(normals[face], P[i]);
            if(val > pVal)
            {
                pVal = val;
                pIdx = static_cast<int>(i);
            }
        }
        if(pIdx < 0)
        {
            continue;
        }
        std::vector<unsigned int> visibleFaces;
        std::vector<unsigned int> invisibleFaces;
        const auto numFaces = (faces.size() / 3UL);
        for(size_t i{0UL}; i < numFaces; ++i)
        {
            if(glm::dot(normals[i], P[pIdx]) > 0)
            {
                visibleFaces.push_back(static_cast<unsigned int>(i));
            }
            else
            {
                invisibleFaces.push_back(static_cast<unsigned int>(i));
            }
        }
        for(unsigned int visible : visibleFaces)
        {
            for(unsigned int invisible : invisibleFaces)
            {
                Edge e;
                if(lix::isAdjacent(&faces[visible * 3], &faces[invisible * 3], e))
                {

                }
            }
        }
    }
}