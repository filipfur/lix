#include "collision.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "polygon.h"
#include "primer.h"

struct Vec3Hash {
    std::size_t operator()(const glm::vec3 &v) const {
        std::hash<float> hasher;
        std::size_t seed = 0;
        seed ^= hasher(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

static std::unordered_map<glm::vec3, glm::vec3, Vec3Hash> minkowskiToDirection;

// #define COLLISION_LOG_TO_FILE
#ifdef COLLISION_LOG_TO_FILE
static std::ofstream logOfs;
static uint32_t logId{0};
#endif

std::ostream &operator<<(std::ostream &os, const glm::vec3 &v) {
    os << '[' << v.x << ' ' << v.y << ' ' << v.z << ']';
    return os;
}

glm::vec3 minkowskiSupportPoint(lix::Shape &a, lix::Shape &b,
                                const glm::vec3 &D) {
    glm::vec3 spA = a.supportPoint(D);
    glm::vec3 spB = b.supportPoint(-D);
    glm::vec3 sp = spA - spB;
    minkowskiToDirection[sp] = D;
    // sp.x = std::max(sp.x, EPSILON);
    // sp.y = std::max(sp.y, EPSILON);
    if (sp.x == 0) {
        sp.x = lix::EPSILON;
    }
    if (sp.y == 0) {
        sp.y = lix::EPSILON;
    }
    return sp;
}

lix::gjk_state emptyCase(lix::Shape &shapeA, lix::Shape &shapeB,
                         std::vector<glm::vec3> &simplex, lix::Collision *,
                         const glm::vec3 &initialDirection) {
    const glm::vec3 B = minkowskiSupportPoint(shapeA, shapeB, initialDirection);
    const glm::vec3 A = minkowskiSupportPoint(shapeA, shapeB, -B); // D=C0

    // glm::vec3 AB = B - A;
    // if (glm::dot(AB, -A) < 0 || glm::dot(AB, AB) < lix::EPSILON)
    if (glm::dot(-A, -B) > 0) {
        return lix::gjk_state::NO_COLLISION; // TODO: Should try more start
                                             // directions of D.
    }

    assert(simplex.size() == 0);

    // simplex.insert(simplex.end(), {B, A});
    simplex.push_back(B);
    simplex.push_back(A);

    assert(simplex.at(0) == B);
    assert(simplex.at(1) == A);

#ifdef COLLISION_LOG_TO_FILE
    logOfs << "shapeA " << shapeA.trs()->translation() << " {"
           << shapeA.trs()->rotation().w << ' ' << shapeA.trs()->rotation().x
           << ' ' << shapeA.trs()->rotation().y << ' '
           << shapeA.trs()->rotation().z << "} " << shapeA.trs()->scale()
           << std::endl;
    logOfs << "shapeB " << shapeB.trs()->translation() << " {"
           << shapeB.trs()->rotation().w << ' ' << shapeB.trs()->rotation().x
           << ' ' << shapeB.trs()->rotation().y << ' '
           << shapeB.trs()->rotation().z << "} " << shapeB.trs()->scale()
           << std::endl;
    logOfs << "supportOfA " << shapeA.supportPoint(initialDirection)
           << std::endl;
    logOfs << "supportOfB " << shapeB.supportPoint(-initialDirection)
           << std::endl;
    glm::vec3 supportOfC = shapeA.supportPoint(initialDirection) -
                           shapeB.supportPoint(-initialDirection);
    logOfs << "supportOfC " << supportOfC << std::endl;
    logOfs << "initialDirection " << initialDirection << std::endl;
    logOfs << "emptyCase " << simplex.at(0) << " " << simplex.at(1)
           << std::endl;
#endif

    return lix::gjk_state::INCREMENTING;
}

lix::gjk_state edgeCase(lix::Shape &shapeA, lix::Shape &shapeB,
                        std::vector<glm::vec3> &simplex,
                        lix::Collision *collision) {
    const glm::vec3 &A = simplex[0];
    const glm::vec3 &B = simplex[1];

    glm::vec3 AB = B - A;

    float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
    assert(t >= 0 && t <= 1);
    glm::vec3 C = A + AB * t;

    glm::vec3 D = minkowskiSupportPoint(shapeA, shapeB, -C);

    float C_dist = glm::dot(C, C);

    if (C_dist < 1.0e-10f) {
        return lix::gjk_state::NO_COLLISION;
    }
    if (C_dist < 1.0e-5f) {
        if (collision) {
            collision->normal = glm::normalize(-C);
            collision->penetrationDepth = glm::sqrt(C_dist);
        }
        return lix::gjk_state::COLLISION;
    }

    if (glm::dot(D, D) < C_dist) {
        // printf("D not closer than C\n");
        return lix::gjk_state::NO_COLLISION;
    }

    simplex.push_back(D);

#ifdef COLLISION_LOG_TO_FILE
    logOfs << "edgeCase " << simplex.at(0) << " " << simplex.at(1) << " "
           << simplex.at(2) << std::endl;
#endif

    return lix::gjk_state::INCREMENTING;
}

lix::gjk_state triangleCase(lix::Shape &shapeA, lix::Shape &shapeB,
                            std::vector<glm::vec3> &simplex) {
    const glm::vec3 &C = simplex[0];
    const glm::vec3 &B = simplex[1];
    const glm::vec3 &A = simplex[2];

    const glm::vec3 AB = B - A;
    const glm::vec3 AC = B - C;
    const glm::vec3 ABC = glm::cross(AB, AC);

    /*if(!lix::pointInTriangle(glm::vec3{0.0f, 0.0f, 0.0f}, A, B, C))
    {
        return lix::gjk_state::NO_COLLISION;
    }*/

    if (!lix::pointInTriangle(glm::vec3{0.0f, 0.0f, 0.0f}, A, B, C)) {
        // const glm::vec3 da = A - D;
        // const glm::vec3 db = B - D;
        float d1 = glm::dot(AB, -A);
        float d2 = glm::dot(AC, -A);
        if (d1 < d2) {
            simplex.erase(simplex.begin() + 1);
            float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
            glm::vec3 T = A + AB * t;
            // assert(t >= 0 && t <= 1);
            if (t < 0 || t > 1) {
                return lix::gjk_state::NO_COLLISION;
            }
            simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, -T));
        } else {
            simplex.erase(simplex.begin() + 0);
            float t = -(glm::dot(AC, A) / glm::dot(AC, AC));
            glm::vec3 T = A + AC * t;
            // assert(t >= 0 && t <= 1);
            if (t < 0 || t > 1) {
                return lix::gjk_state::NO_COLLISION;
            }
            simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, -T));
        }
        return lix::gjk_state::INCREMENTING;
    }

    float dist = glm::dot(ABC, -A);
    assert(dist != 0.0f);
    if (dist > 0) // Triangle above the origin
    {
        simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, ABC));
        std::swap(simplex[0], simplex[1]); // B <-> C
        // return -1;
    } else if (dist < 0) {
        simplex.push_back(minkowskiSupportPoint(shapeA, shapeB, -ABC));
        // std::swap(simplex[0], simplex[1]); // B <-> C
        // return -1;
    }

#ifdef COLLISION_LOG_TO_FILE
    logOfs << "triangleCase " << simplex.at(0) << " " << simplex.at(1) << " "
           << simplex.at(2) << " " << simplex.at(3) << std::endl;
#endif

    return lix::gjk_state::INCREMENTING;
}

lix::gjk_state tetrahedronCase(lix::Shape &shapeA, lix::Shape &shapeB,
                               std::vector<glm::vec3> &simplex,
                               lix::Collision *collision) {
    const glm::vec3 &D = simplex[0];
    const glm::vec3 &C = simplex[1];
    const glm::vec3 &B = simplex[2];
    const glm::vec3 &A = simplex[3];

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
    if (l0 >= 0) {
        simplex.erase(simplex.begin());
        glm::vec3 E = minkowskiSupportPoint(shapeA, shapeB, ABC);
        if (glm::dot(ABC, -E) > 0) {
            return lix::gjk_state::NO_COLLISION;
        }
        simplex.push_back(E);
        return lix::gjk_state::INCREMENTING;
    }
    float l1 = glm::dot(ACD, -A);
    if (l1 >= 0) {
        simplex.erase(simplex.begin() + 2);
        glm::vec3 E = minkowskiSupportPoint(shapeA, shapeB, ACD);
        if (glm::dot(ACD, -E) > 0) {
            return lix::gjk_state::NO_COLLISION;
        }
        simplex.push_back(E);
        return lix::gjk_state::INCREMENTING;
    }
    float l2 = glm::dot(ADB, -A);
    if (l2 >= 0) {
        simplex.erase(simplex.begin() + 1);
        glm::vec3 E = minkowskiSupportPoint(shapeA, shapeB, ADB);
        if (glm::dot(ADB, -E) > 0) {
            return lix::gjk_state::NO_COLLISION;
        }
        simplex.push_back(E);
        return lix::gjk_state::INCREMENTING;
    }
    float l3 = glm::dot(BDC, -B);
    if (l3 >= 0) {
        simplex.erase(simplex.begin() + 3);
        glm::vec3 E = minkowskiSupportPoint(shapeA, shapeB, BDC);
        if (glm::dot(BDC, -E) > 0) {
            return lix::gjk_state::NO_COLLISION;
        }
        simplex.push_back(E);
        std::swap(simplex[3], simplex[2]);
        return lix::gjk_state::INCREMENTING;
    }

    glm::vec3 R = ABC + ACD + ADB + BDC;
    float len = R.x * R.x + R.y * R.y + R.z * R.z;
    if (len >= lix::EPSILON) {
        std::cerr << "total length of tetrahedron normals where too big, len="
                  << len << std::endl;
        exit(1);
    }

    // std::cout << "len=" << len << std::endl;
    if (collision) {
        float minDistance = l0; // min distance is negative
        glm::vec3 minNormal = -ABC;

        if (l1 > minDistance) {
            minNormal = -ACD;
            minDistance = l1;
        }
        if (l2 > minDistance) {
            minNormal = -ADB;
            minDistance = l2;
        }
        if (l3 > minDistance) {
            minNormal = -BDC;
            minDistance = l3;
        }

        float d =
            minDistance / glm::length(ABC); // glm::sqrt(glm::dot(ABC, ABC));

        collision->penetrationDepth = -d;
        collision->normal = glm::normalize(minNormal);
    }

#ifdef COLLISION_LOG_TO_FILE
    logOfs << "tetrahedronCase " << simplex.at(0) << " " << simplex.at(1) << " "
           << simplex.at(2) << " " << simplex.at(3) << std::endl;
#endif

    return lix::gjk_state::COLLISION;
}

bool lix::collides(lix::Shape &a, lix::Shape &b, lix::Collision *collision) {
    auto &simpA = a.simplified() ? *a.simplified() : a;
    auto &simpB = b.simplified() ? *b.simplified() : b;

    if (simpA.test(simpB)) {
        std::vector<glm::vec3> simplex;
        const glm::vec3 D =
            glm::normalize(b.trs()->translation() - a.trs()->translation());
        if (lix::gjk(a, b, simplex, D, collision)) {
            if (!lix::epa(a, b, simplex, collision)) {
                printf("error: epa failed\n");
                return false;
            }
            return true;
        }
    }
    return false;
}

bool lix::gjk(Shape &shapeA, Shape &shapeB, std::vector<glm::vec3> &simplex,
              const glm::vec3 &initialDirection, lix::Collision *collision) {
    gjk_init();

    lix::gjk_state rval{lix::gjk_state::INCREMENTING};
    for (size_t i{0UL}; (i < 64UL) && (rval == gjk_state::INCREMENTING); ++i) {
        rval =
            gjk_increment(shapeA, shapeB, simplex, initialDirection, collision);
    }
    assert(rval != gjk_state::ERROR);
    return rval == gjk_state::COLLISION;
}

void lix::gjk_init() {
    minkowskiToDirection.clear();
#ifdef COLLISION_LOG_TO_FILE
    if (logOfs.is_open()) {
        logOfs.close();
        ++logId;
    }
    logOfs.open("log_" + std::to_string(logId) + ".txt");
#endif
}

lix::gjk_state lix::gjk_increment(Shape &shapeA, Shape &shapeB,
                                  std::vector<glm::vec3> &simplex,
                                  const glm::vec3 &initialDirection,
                                  lix::Collision *collision) {
    lix::gjk_state rval{lix::gjk_state::ERROR};
    switch (simplex.size()) {
    case 0UL:
        rval = emptyCase(shapeA, shapeB, simplex, collision, initialDirection);
        break;
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
        rval = gjk_state::ERROR;
        printf("error: gjk unknown number of simplex points\n");
        break;
    }
    return rval;
}

std::pair<std::list<lix::Face>::iterator, float>
findClosestFace(lix::ConvexHull &ch) {
    float minDistance =
        glm::dot(ch.begin()->normal, ch.begin()->half_edge->vertex);
    auto minFaceIt = ch.begin();
    for (auto it = ++ch.begin(); it != ch.end(); ++it) {
        float distance = glm::dot(it->normal, it->half_edge->vertex);
        assert(distance > 0);
        if (distance < minDistance) {
            minDistance = distance;
            minFaceIt = it;
        }
    }
    return {minFaceIt, minDistance};
}

bool lix::epa(Shape &shapeA, Shape &shapeB,
              const std::vector<glm::vec3> &simplex,
              lix::Collision *collision) {
    for (size_t i{0}; i < simplex.size(); ++i) {
        for (size_t j{0}; j < simplex.size(); ++j) {
            if (i == j)
                continue;
            assert(!isSameVertex(simplex[i], simplex[j]));
        }
    }

    lix::ConvexHull ch{simplex};

    for (size_t i{0}; i < 50; ++i) {
        if (epa_increment(shapeA, shapeB, ch, collision)) {
            return true;
        }
    }
    return false;
}

bool lix::epa_increment(Shape &shapeA, Shape &shapeB, lix::ConvexHull &ch,
                        lix::Collision *collision) {
    auto [minFaceIt, minDistance] = findClosestFace(ch);
    glm::vec3 sp = minkowskiSupportPoint(
        shapeA, shapeB,
        minFaceIt->normal); // shapeA.supportPoint(minFaceIt->normal) -
                            // shapeB.supportPoint(-minFaceIt->normal);
    float sDistance = glm::dot(minFaceIt->normal, sp);
    // printf("sDistance=%.3f minDistance=%.3f\n", sDistance, minDistance);
    if (glm::abs(sDistance - minDistance) < 0.0001f) {
        const glm::vec3 &a = minFaceIt->half_edge->vertex;
        const glm::vec3 &b = minFaceIt->half_edge->next->vertex;
        const glm::vec3 &c = minFaceIt->half_edge->next->next->vertex;

        if (collision) {
            glm::vec3 aa = getSupportPointOfA(shapeA, a);
            glm::vec3 bb = getSupportPointOfA(shapeA, b);
            glm::vec3 cc = getSupportPointOfA(shapeA, c);

            bool isSameAB = isSameVertex(aa, bb);
            bool isSameBC = isSameVertex(bb, cc);
            bool isSameCA = isSameVertex(cc, aa);
            if (isSameAB && isSameBC) {
                collision->contactPoint = aa;
            } else if (isSameAB) {
                const glm::vec3 bc = c - b;
                float t = -(glm::dot(bc, b) / glm::dot(bc, bc));
                collision->contactPoint = bb + (cc - bb) * t;
            } else if (isSameBC) {
                const glm::vec3 ab = b - a;
                float t = -(glm::dot(ab, a) / glm::dot(ab, ab));
                collision->contactPoint = aa + (bb - aa) * t;
            } else if (isSameCA) {
                const glm::vec3 bc = c - b;
                float t = -(glm::dot(bc, b) / glm::dot(bc, bc));
                collision->contactPoint = bb + (cc - bb) * t;
            } else {
                glm::vec3 bary =
                    barycentric(minFaceIt->normal * minDistance, a, b, c);
                collision->contactPoint =
                    aa * bary[0] + bb * bary[1] + cc * bary[2];
            }

            collision->a = aa;
            collision->b = bb;
            collision->c = cc;

            collision->normal = -minFaceIt->normal;
            collision->penetrationDepth =
                minDistance; // glm::dot(minFaceIt->normal, a);
        }
        // printf("converged on iteration: %zu\n", i);
        return true;
    } else {
        assert(ch.addPoint(sp));
        // printf("adding %.1f %.1f %.1f\n", sp.x, sp.y, sp.z);
    }
    return false;
}

glm::vec3 lix::getSupportPointOfA(lix::Shape &shape,
                                  const glm::vec3 &minkowskiSP) {
    return shape.supportPoint(minkowskiToDirection.at(minkowskiSP));
}

glm::vec3 lix::getSupportPointOfB(lix::Shape &shape,
                                  const glm::vec3 &minkowskiSP) {
    return shape.supportPoint(-minkowskiToDirection.at(minkowskiSP));
}