#pragma once

#include "glm/glm.hpp"

namespace impact
{
    float sign(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
    bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
}