#include "collision.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

const float EPSILON{0.00001f};

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

float sign(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
}

bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
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
        if(pointInTriangle(x, p, *ret, q))
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

void impact::quickHull(const std::vector<glm::vec3>& s, std::vector<glm::vec3>& ch)
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

glm::vec3 support(impact::Shape& a, impact::Shape& b, const glm::vec3& D, glm::vec3& spa, glm::vec3& spb)
{
    spa = a.supportPoint(D);
    spb = b.supportPoint(-D);
    glm::vec3 sp = spa - spb;
    if(sp.x == 0)
    {
        sp.x = EPSILON;
    }
    if(sp.y == 0)
    {
        sp.y = EPSILON;
    }
    return sp;
}

bool isNearby(const glm::vec3& A, const glm::vec3& B)
{
    glm::vec3 D = B - A;
    return (D.x * D.x + D.y * D.y + D.z * D.z) < 0.00001f;
}

bool isLine(const std::vector<glm::vec3>& s)
{
    const glm::vec3& A = s.at(0);
    const glm::vec3& B = s.at(1);
    const glm::vec3& C = s.at(2);
    return isNearby(A, B) || isNearby(B, C) || isNearby(C, A);
}

bool impact::gjk(Shape& a, Shape& b, std::vector<glm::vec3>& simplex,
    const glm::vec3& initialDirection)
{    
    glm::vec3 a1;
    glm::vec3 b1;
    glm::vec3 c1;
    glm::vec3 a2;
    glm::vec3 b2;
    glm::vec3 c2;

    glm::vec3 C = support(a, b, initialDirection, c1, c2);

    glm::vec3 D = -C; // CO

    glm::vec3 B = support(a, b, D, b1, b2); // D=C0

    glm::vec3 BO = -B;
    glm::vec3 BC = C - B;

    // Check if the CO and OB are pointing in the same "general" direction. Otherwise the origin was not crossed.
    if (glm::dot(BC, BO) < 0) // CO dot OB. From the perspective of C is point B further away than the origin.
    {
        //std::cout << "did not pass origin." << std::endl;
        return false; // TODO: Should try more start directions of D.
    }

    D = glm::cross(glm::cross(BC, BO), BC);

    glm::vec3 A = support(a, b, D, a1, a2);
    bool rval{false};

    for(size_t i{0}; i < 64; ++i)
    {
        glm::vec3 AO = -A;
        glm::vec3 AB = B - A;
        glm::vec3 AC = C - A;

        glm::vec3 ABC = glm::cross(AB, AC);

        glm::vec3 ABperp = glm::cross(AB, ABC);
        glm::vec3 ACperp = glm::cross(ABC, AC);

        if(glm::dot(ABperp, AO) > 0) {
            //std::cout << "line is outside AB" << std::endl;
            C = support(a, b, ABperp, c1, c2);
            std::swap(B, C);
            std::swap(b1, c1);
            std::swap(b2, c2);
            if(glm::dot(B - A, AO) < 0) // Really?
            {
                //std::cout << "AB perp not pass the origin." << std::endl;
                return false;
            }
        }
        else if(glm::dot(ACperp, AO) > 0) {
            //std::cout << "line is outside AC" << std::endl;
            B = support(a, b, ACperp, b1, b2);
            std::swap(B, C);
            std::swap(b1, c1);
            std::swap(b2, c2);
            if(glm::dot(C - A, AO) < 0) // Really?
            {
                //std::cout << "AC perp not pass the origin." << std::endl;
                return false;
            }
        }
        else {
            if(pointInTriangle(glm::vec3{0.0f, 0.0f, 0.0f}, A, B, C))
            {
                simplex.insert(simplex.end(), {A, B, C});
                rval = true;
                break;
            }
        }
    }

    return rval;
}

struct Edge
{
    float distance;
    glm::vec3 normal;
    int a;
    int b;
};

bool findClosestEdge(std::vector<glm::vec3>& simplex, Edge& closest)
{
    bool rval = false;
    closest.distance = FLT_MAX;
    for(size_t i{0}; i < simplex.size(); ++i)
    {
        int j = (i + 1 == simplex.size()) ? 0 : i + 1;
        glm::vec3 A = simplex.at(i);
        glm::vec3 B = simplex.at(j);
        glm::vec3 AB = B - A;   

        glm::vec3 N = glm::cross(AB, glm::cross(A, AB));
        if(glm::dot(N, A) < 0)
        {
            N = -N;
        }
        N = glm::normalize(N);
        float f = glm::dot(N, A);
        if(f < closest.distance)
        {
            closest.distance = f;
            closest.normal = N;
            closest.a = i;
            closest.b = j;
            rval = true;
        }
    }
    return rval;
}

bool impact::epa(Shape& shapeA, Shape& shapeB, std::vector<glm::vec3>& simplex,
    glm::vec3& collisionVector, float& penetration)
{
    bool rval{false};

    while(true)
    {
        if(simplex.size() > 64)
        {
            return false;
        }
        Edge e;
        e.a = 0;
        e.b = 1;
        e.distance = FLT_MAX;
        if(!findClosestEdge(simplex, e))
        {
            e.normal = -simplex.at(0);
        }
        glm::vec3 a, b;
        glm::vec3 p = support(shapeA, shapeB, e.normal, a, b);

        float f = glm::dot(p, e.normal);

        if(glm::abs(f - e.distance) < 0.00001f)
        {
            collisionVector = e.normal;
            penetration = f;
            rval = true;
            break;
        }
        else
        {
            simplex.insert(simplex.begin() + e.b, p);
        }
    }
    return rval;
}