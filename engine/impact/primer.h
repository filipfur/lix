#pragma once

#include <vector>

#include "glm/glm.hpp"

namespace lix {
using Edge = std::pair<unsigned int, unsigned int>;
inline static constexpr float EPSILON{0.00001f};

float sign(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
bool pointInTriangle2d(const glm::vec3 &p, const glm::vec3 &a,
                       const glm::vec3 &b, const glm::vec3 &c);
bool pointInTriangle(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b,
                     const glm::vec3 &c);

glm::vec3 barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);

inline bool isSameVertex(const glm::vec3 &a, const glm::vec3 &b) {
    const glm::vec3 c = b - a;
    return glm::dot(c, c) < EPSILON;
}

bool containsVertex(const std::vector<glm::vec3> &s, const glm::vec3 &v);

std::vector<glm::vec3> uniqueVertices(const std::vector<glm::vec3> &s);

std::pair<glm::vec3, glm::vec3> extremePoints(const std::vector<glm::vec3> &s);
std::vector<glm::vec3> minimumBoundingBox(const glm::vec3 &min,
                                          const glm::vec3 &max);

std::pair<int, float> indexAlongDirection(const std::vector<glm::vec3> &s,
                                          const glm::vec3 &D);

bool isAdjacent(const unsigned int *a, const unsigned int *b, Edge &edge);

glm::quat directionToQuat(const glm::vec3 &direction);

glm::vec3 quatToDirection(const glm::quat &quat);
} // namespace lix