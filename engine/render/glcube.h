#pragma once

#include <array>
#include <glm/glm.hpp>

namespace lix
{
    extern const std::array<unsigned int, 36> cube_indices;
    extern const std::array<glm::vec3, 8> cube_positions;

    std::vector<glm::vec3> cube_corner_points(const glm::vec3& min, const glm::vec3& max);

    std::pair<std::vector<float>, std::vector<unsigned int>> cubes_at_points(const std::vector<glm::vec3>& points, float radii=0.01f);
}