#include "collision.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "convexhull.h"
#include "polygon.h"
#include "primer.h"

static uint32_t supportIndex{0};
static std::vector<glm::vec3> supportA;
static std::vector<glm::vec3> supportB;

lix::Vertex minkowskiSupportPoint(lix::Shape& a, lix::Shape& b, const glm::vec3& D)
{
    glm::vec3 spA = a.supportPoint(D);
    glm::vec3 spB = b.supportPoint(-D);
    glm::vec3 sp = spA - spB;
    supportA.push_back(spA);
    supportB.push_back(spB);
    //sp.x = std::max(sp.x, EPSILON);
    //sp.y = std::max(sp.y, EPSILON);
    if(sp.x == 0)
    {
        sp.x = lix::EPSILON;
    }
    if(sp.y == 0)
    {
        sp.y = lix::EPSILON;
    }
    return {supportIndex++, sp};
}

int edgeCase(lix::Shape& shapeA, lix::Shape& shapeB, std::vector<lix::Vertex>& simplex, lix::Collision* collision)
{
    const lix::Vertex& A = simplex[0];
    const lix::Vertex& B = simplex[1];

    glm::vec3 AB = B.position - A.position;

    float t = -(glm::dot(AB, A.position) / glm::dot(AB, AB));
    glm::vec3 C = A.position + AB * t;

    lix::Vertex D = minkowskiSupportPoint(shapeA, shapeB, -C);

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

    if(glm::dot(D.position, D.position) < C_dist)
    {
        //printf("D not closer than C\n");
        return -1;
    }

    simplex.push_back(D);
    return 0;
}

int triangleCase(lix::Shape& shapeA, lix::Shape& shapeB, std::vector<lix::Vertex>& simplex)
{
    const lix::Vertex& C = simplex[0];
    const lix::Vertex& B = simplex[1];
    const lix::Vertex& A = simplex[2];

    const glm::vec3 AB = B.position - A.position;
    const glm::vec3 AC = B.position - C.position;
    const glm::vec3 ABC = glm::cross(AB, AC);

    if(glm::dot(ABC, -A.position) > 0) // Triangle above the origin
    {
        simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, ABC));
        std::swap(simplex[0], simplex[1]); // B <-> C
        std::swap(supportA[0], supportA[1]);
        std::swap(supportB[0], supportB[1]);
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

int tetrahedronCase(lix::Shape& /*shapeA*/, lix::Shape& /*shapeB*/, std::vector<lix::Vertex>& simplex, lix::Collision* collision)
{
    const lix::Vertex& D = simplex[0];
    const lix::Vertex& C = simplex[1];
    const lix::Vertex& B = simplex[2];
    const lix::Vertex& A = simplex[3];

    const glm::vec3 AB = B.position - A.position;
    const glm::vec3 AC = C.position - A.position;
    const glm::vec3 AD = D.position - A.position;
    const glm::vec3 BC = C.position - B.position;
    const glm::vec3 BD = D.position - B.position;

    const glm::vec3 ABC = glm::cross(AB, AC);
    const glm::vec3 ACD = glm::cross(AC, AD);
    const glm::vec3 ADB = glm::cross(AD, AB);
    const glm::vec3 BDC = glm::cross(BD, BC);

    float l0 = glm::dot(ABC, -A.position);
    if(l0 >= 0)
    {
        return -1;
    }
    float l1 = glm::dot(ACD, -A.position);
    if(l1 >= 0)
    {
        return -1;
    }
    float l2 = glm::dot(ADB, -A.position);
    if(l2 >= 0)
    {
        return -1;
    }
    float l3 = glm::dot(BDC, -B.position);
    if(l3 >= 0)
    {
        return -1;
    }

    glm::vec3 R = ABC + ACD + ADB + BDC;
    float len = R.x * R.x + R.y * R.y + R.z * R.z;
    if(len >= lix::EPSILON)
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

bool lix::gjk(Shape& shapeA, Shape& shapeB, std::vector<lix::Vertex>& simplex,
    const glm::vec3& initialDirection, lix::Collision* collision)
{
    supportIndex = 0;
    supportA.clear();
    supportB.clear();

    lix::Vertex B = minkowskiSupportPoint(shapeA, shapeB, initialDirection);

    lix::Vertex A = minkowskiSupportPoint(shapeA, shapeB, -B.position); // D=C0

    glm::vec3 AO = -A.position;
    glm::vec3 AB = B.position - A.position;

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

std::pair<std::list<lix::Face>::iterator, float> findClosestFace(lix::ConvexHull& ch)
{
    float minDistance = glm::dot(ch.begin()->normal, ch.begin()->half_edge->vertex.position);
    auto minFaceIt = ch.begin();
    for(auto it = ++ch.begin(); it != ch.end(); ++it)
    {
        float distance = glm::dot(it->normal, it->half_edge->vertex.position);
        assert(distance > 0);
        if(distance < minDistance)
        {
            minDistance = distance;
            minFaceIt = it;
        }
    }
    return {minFaceIt, minDistance};
}

bool lix::epa(Shape& shapeA, Shape& shapeB, const std::vector<lix::Vertex>& simplex, lix::Collision* collision)
{
    for(size_t i{0}; i < simplex.size(); ++i)
    {
        for(size_t j{0}; j < simplex.size(); ++j)
        {
            if(i == j) continue;
            assert(!isSameVertex(simplex[i].position, simplex[j].position));
        }
    }

    lix::ConvexHull ch{simplex};

    for(size_t i{0}; i < 50; ++i)
    {
        auto [minFaceIt, minDistance] = findClosestFace(ch);
        lix::Vertex sp = minkowskiSupportPoint(shapeA, shapeB, minFaceIt->normal);// shapeA.supportPoint(minFaceIt->normal) - shapeB.supportPoint(-minFaceIt->normal);
        float sDistance = glm::dot(minFaceIt->normal, sp.position);
        //printf("sDistance=%.3f minDistance=%.3f\n", sDistance, minDistance);
        if (glm::abs(sDistance - minDistance) < 0.0001f)
        {
            const Vertex& a = minFaceIt->half_edge->vertex;
            const Vertex& b = minFaceIt->half_edge->next->vertex;
            const Vertex& c = minFaceIt->half_edge->next->next->vertex;

            if(collision)
            {
                glm::vec3 aa = supportA.at(a.id);
                glm::vec3 bb = supportA.at(b.id);
                glm::vec3 cc = supportA.at(c.id);

                bool isSameAB = isSameVertex(aa, bb);
                bool isSameBC = isSameVertex(bb, cc);
                bool isSameCA = isSameVertex(cc, aa);
                if(isSameAB && isSameBC)
                {
                    collision->contactPoint = aa;
                }
                else if(isSameAB)
                {
                    const glm::vec3 bc = c.position - b.position;
                    float t = -(glm::dot(bc, b.position) / glm::dot(bc, bc));
                    collision->contactPoint = bb + (cc - bb) * t;
                }
                else if(isSameBC)
                {
                    const glm::vec3 ab = b.position - a.position;
                    float t = -(glm::dot(ab, a.position) / glm::dot(ab, ab));
                    collision->contactPoint = aa + (bb - aa) * t;
                }
                else if(isSameCA)
                {
                    const glm::vec3 bc = c.position - b.position;
                    float t = -(glm::dot(bc, b.position) / glm::dot(bc, bc));
                    collision->contactPoint = bb + (cc - bb) * t;
                }
                else
                {
                    glm::vec3 bary = barycentric(minFaceIt->normal * minDistance,
                        a.position, b.position, c.position);
                    collision->contactPoint = aa * bary[0]
                        + bb * bary[1]
                        + cc * bary[2];
                }

                collision->a = aa;
                collision->b = bb;
                collision->c = cc;
                collision->collisionNormal = -minFaceIt->normal;
                collision->penetrationDepth = glm::dot(minFaceIt->normal, a.position);
            }
            //printf("converged on iteration: %zu\n", i);
            return true;
        }
        else
        {
            assert(ch.addPoint(sp));
            //printf("adding %.1f %.1f %.1f\n", sp.x, sp.y, sp.z);
        }
    }
    return false;
}