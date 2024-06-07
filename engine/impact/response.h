#pragma once

#include "glm/glm.hpp"

namespace lix
{
    float impulse(
        float restitution,
        float m1_inv,
        float m2_inv,
        const glm::mat3& I1_inv,
        const glm::mat3& I2_inv,
        const glm::vec3& r1,
        const glm::vec3& r2,
        const glm::vec3& n, // collision normal
        glm::vec3& v1,
        glm::vec3& v2,
        glm::vec3& w1,
        glm::vec3& w2)
    {
        const glm::vec3 vrel = (v2 + glm::cross(w2, r2)) - (v1 + glm::cross(w1, r1));
        
        float numerator = glm::dot(-(1 + restitution) * vrel, n);
        glm::vec3 n1 = glm::cross(r1, n);
        glm::vec3 n2 = glm::cross(r2, n);
        float denominator = m1_inv + m2_inv + glm::dot(n1, I1_inv * n1) + glm::dot(n2, I2_inv * n2);

        float j = numerator / denominator;
        glm::vec3 J = n * j;

        w1 += glm::cross(r1, J) * I1_inv;
        w2 += glm::cross(r2, J) * I2_inv;

        return j;
    }
}