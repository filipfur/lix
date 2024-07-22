#include "glcube.h"

const std::array<unsigned int, 36> lix::cube_indices = {
    0, 1, 3,
    3, 1, 2,

    3, 2, 4,
    4, 2, 5,

    7, 4, 5,
    7, 5, 6,

    0, 7, 1,
    7, 6, 1,
    
    2, 1, 6,
    2, 6, 5,

    3, 7, 0,
    4, 7, 3
};


static const glm::vec3 max{1.0f, 1.0f, 1.0f};
static const glm::vec3 min{-1.0f, -1.0f, -1.0f};

const std::array<glm::vec3, 8> lix::cube_positions{
    glm::vec3{max.x, min.y, min.z},
    glm::vec3{min.x, min.y, min.z},
    glm::vec3{min.x, max.y, min.z},
    glm::vec3{max.x, max.y, min.z},

    glm::vec3{max.x, max.y, max.z},
    glm::vec3{min.x, max.y, max.z},
    glm::vec3{min.x, min.y, max.z},
    glm::vec3{max.x, min.y, max.z},
};

std::vector<glm::vec3> lix::cube_corner_points(const glm::vec3& min, const glm::vec3& max)
{
    return {
        {max.x, min.y, min.z},
        {min.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},

        {max.x, max.y, max.z},
        {min.x, max.y, max.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
    };
}

std::pair<std::vector<float>, std::vector<unsigned int>> lix::cubes_at_points(const std::vector<glm::vec3>& points, float radii)
{
    std::vector<float> vs;
    std::vector<unsigned int> is;
    vs.reserve(points.size() * lix::cube_positions.size() * 3);
    is.reserve(points.size() * lix::cube_indices.size());

    for(size_t i{0}; i < points.size(); ++i)
    {
        const auto& p = points.at(i);
        for(const auto& v : lix::cube_positions)
        {
            glm::vec3 d = p + v * radii;
            vs.insert(vs.end(), {
                d.x, d.y, d.z
            });
        }
        for(const auto& j : lix::cube_indices)
        {
            is.push_back(static_cast<unsigned int>(8 * i + j));
        }
    }
    return { vs, is };
}