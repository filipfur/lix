#include "collision.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "primer.h"

inline static constexpr float EPSILON{0.00001f};

glm::vec3 minkowskiSupportPoint(lix::Shape& a, lix::Shape& b, const glm::vec3& D)
{
    glm::vec3 sp = a.supportPoint(D) - b.supportPoint(-D);
    //sp.x = std::max(sp.x, EPSILON);
    //sp.y = std::max(sp.y, EPSILON);
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

bool lix::gjk2d(Shape& shapeA, Shape& shapeB, std::vector<glm::vec3>& simplex,
    const glm::vec3& initialDirection)
{
    const glm::vec3 O{0.0f, 0.0f, 0.0f};
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
            if(lix::pointInTriangle(O, A, B, C))
            {
                simplex.insert(simplex.end(), {A, B, C});
                rval = true;
                break;
            }
        }
    }
    return rval;
}

int edgeCase(lix::Shape& shapeA, lix::Shape& shapeB, std::vector<glm::vec3>& simplex, lix::Collision* collision)
{
    const glm::vec3& A = simplex[0];
    const glm::vec3& B = simplex[1];

    glm::vec3 AB = B - A;

    float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
    glm::vec3 C = A + AB * t;

    glm::vec3 D = minkowskiSupportPoint(shapeA, shapeB, -C);

    float C_dist = glm::dot(C, C);

    if(C_dist < 1.0e-10f)
    {
        return -1;
    }
    if(C_dist < 1.0e-5f)
    {
        if(collision)
        {
            collision->collisionNormal = glm::normalize(-C);
            collision->penetrationDepth = glm::sqrt(C_dist);
        }
        return 1;
    }

    if(glm::dot(D, D) < C_dist)
    {
        //printf("D not closer than C\n");
        return -1;
    }

    simplex.push_back(D);
    return 0;
}

int triangleCase(lix::Shape& shapeA, lix::Shape& shapeB, std::vector<glm::vec3>& simplex)
{
    const glm::vec3& C = simplex[0];
    const glm::vec3& B = simplex[1];
    const glm::vec3& A = simplex[2];

    const glm::vec3 AB = B - A;
    const glm::vec3 AC = B - C;
    const glm::vec3 ABC = glm::cross(AB, AC);

    if(glm::dot(ABC, -A) > 0) // Triangle above the origin
    {
        simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, ABC));
        std::swap(simplex[0], simplex[1]); // B <-> C
        shapeA.swapIndices(0, 1);
        shapeB.swapIndices(0, 1);
        //return -1;
    }
    else
    {
        simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, -ABC));
        //std::swap(simplex[0], simplex[1]); // B <-> C
        //return -1;
    }

    return 0;
}

int tetrahedronCase(lix::Shape& /*shapeA*/, lix::Shape& /*shapeB*/, std::vector<glm::vec3>& simplex, lix::Collision* collision)
{
    const glm::vec3& D = simplex[0];
    const glm::vec3& C = simplex[1];
    const glm::vec3& B = simplex[2];
    const glm::vec3& A = simplex[3];

    const glm::vec3 AB = B - A;
    const glm::vec3 AC = C - A;
    const glm::vec3 AD = D - A;
    const glm::vec3 BC = C - B;
    const glm::vec3 BD = D - B;

    const glm::vec3 ABC = glm::cross(AB, AC);
    const glm::vec3 ACD = glm::cross(AC, AD);
    const glm::vec3 ADB = glm::cross(AD, AB);
    const glm::vec3 BDC = glm::cross(BD, BC);

    float l0 = glm::dot(ABC, -A);
    if(l0 >= 0)
    {
        return -1;
    }
    float l1 = glm::dot(ACD, -A);
    if(l1 >= 0)
    {
        return -1;
    }
    float l2 = glm::dot(ADB, -A);
    if(l2 >= 0)
    {
        return -1;
    }
    float l3 = glm::dot(BDC, -B);
    if(l3 >= 0)
    {
        return -1;
    }

    glm::vec3 R = ABC + ACD + ADB + BDC;
    float len = R.x * R.x + R.y * R.y + R.z * R.z;
    if(len >= EPSILON)
    {
        std::cerr << "total length of tetrahedron normals where too big, len=" << len << std::endl;
        exit(1);
    }

    //std::cout << "len=" << len << std::endl;
    if(collision)
    {
        float minDistance = l0; // min distance is negative
        glm::vec3 minNormal = -ABC;

        if(l1 > minDistance)
        {
            minNormal = -ACD;
            minDistance = l1;
        }
        if(l2 > minDistance)
        {
            minNormal = -ADB;
            minDistance = l2;
        }
        if(l3 > minDistance)
        {
            minNormal = -BDC;
            minDistance = l3;
        }

        float d = minDistance / glm::length(ABC); //glm::sqrt(glm::dot(ABC, ABC));

        collision->penetrationDepth = -d;
        collision->collisionNormal = glm::normalize(minNormal);
    }

    return 1;
}

bool lix::gjk(Shape& shapeA, Shape& shapeB, std::vector<glm::vec3>& simplex,
    const glm::vec3& initialDirection, lix::Collision* collision)
{
    shapeA.clearIndices();
    shapeB.clearIndices();
    //const glm::vec3 O{0.0f, 0.0f, 0.0f};
    glm::vec3 B = minkowskiSupportPoint(shapeA, shapeB, initialDirection);

    glm::vec3 A = minkowskiSupportPoint(shapeA, shapeB, -B); // D=C0

    glm::vec3 AO = -A;
    glm::vec3 AB = B - A;

    // Check if the CO and OB are pointing in the same "general" direction. Otherwise the origin was not crossed.
    if (glm::dot(AB, AO) < 0 || glm::dot(AB, AB) < EPSILON) // CO dot OB. From the perspective of C is point B further away than the origin.
    {
        //std::cout << "did not pass origin." << std::endl;
        return false; // TODO: Should try more start directions of D.
    }

    simplex.insert(simplex.end(), {B, A});
    
    int rval{0};
    for(size_t i{0UL}; (i < 64UL) && (rval == 0); ++i)
    {
        switch(simplex.size())
        {
        case 2UL:
            rval = edgeCase(shapeA, shapeB, simplex, collision);
            break;
        case 3UL:
            rval = triangleCase(shapeA, shapeB, simplex);
            break;
        case 4UL:
            rval = tetrahedronCase(shapeA, shapeB, simplex, collision);
            break;
        default:
            rval = -1;
            exit(1);
            break;
        }
    }
    return rval > 0;
}

struct EdgeNormal
{
    float distance;
    glm::vec3 normal;
    int a;
    int b;
};

bool findClosestEdge(std::vector<glm::vec3>& simplex, EdgeNormal& closest)
{
    bool rval = false;
    closest.distance = FLT_MAX;
    for(size_t i{0}; i < simplex.size(); ++i)
    {
        size_t j = (i + 1 == simplex.size()) ? 0 : i + 1;
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
            closest.a = static_cast<int>(i);
            closest.b = static_cast<int>(j);
            rval = true;
        }
    }
    return rval;
}

bool lix::epa(Shape& shapeA, Shape& shapeB, std::vector<glm::vec3>& simplex,
    glm::vec3& collisionVector, float& penetration)
{
    bool rval{false};

    while(true)
    {
        if(simplex.size() > 64)
        {
            return false;
        }
        EdgeNormal e;
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