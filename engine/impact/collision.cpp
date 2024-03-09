#include "collision.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "primer.h"

const float EPSILON{0.00001f};

glm::vec3 minkowskiSupportPoint(impact::Shape& a, impact::Shape& b, const glm::vec3& D)
{
    glm::vec3 sp = a.supportPoint(D) - b.supportPoint(-D);
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

bool impact::gjk(Shape& shapeA, Shape& shapeB, std::vector<glm::vec3>& simplex,
    const glm::vec3& initialDirection)
{    
    glm::vec3 C = minkowskiSupportPoint(shapeA, shapeB, initialDirection);

    glm::vec3 D = -C; // CO

    glm::vec3 B = minkowskiSupportPoint(shapeA, shapeB, D); // D=C0

    glm::vec3 BO = -B;
    glm::vec3 BC = C - B;

    // Check if the CO and OB are pointing in the same "general" direction. Otherwise the origin was not crossed.
    if (glm::dot(BC, BO) < 0) // CO dot OB. From the perspective of C is point B further away than the origin.
    {
        //std::cout << "did not pass origin." << std::endl;
        return false; // TODO: Should try more start directions of D.
    }

    D = glm::cross(glm::cross(BC, BO), BC);

    glm::vec3 A = minkowskiSupportPoint(shapeA, shapeB, D);
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
            C = minkowskiSupportPoint(shapeA, shapeB, ABperp);
            std::swap(B, C);
            if(glm::dot(B - A, AO) < 0) // Really?
            {
                //std::cout << "AB perp not pass the origin." << std::endl;
                return false;
            }
        }
        else if(glm::dot(ACperp, AO) > 0) {
            //std::cout << "line is outside AC" << std::endl;
            B = minkowskiSupportPoint(shapeA, shapeB, ACperp);
            std::swap(B, C);
            if(glm::dot(C - A, AO) < 0) // Really?
            {
                //std::cout << "AC perp not pass the origin." << std::endl;
                return false;
            }
        }
        else {
            if(impact::pointInTriangle(glm::vec3{0.0f, 0.0f, 0.0f}, A, B, C))
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
        glm::vec3 p = minkowskiSupportPoint(shapeA, shapeB, e.normal);

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