#include "glgeometry.h"

#include "glm/glm.hpp"
#include <cmath>

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


std::pair<std::vector<float>, std::vector<unsigned int>> lix::sphere(unsigned int segments, unsigned int discs)
{
    std::vector<float> vs;
    std::vector<unsigned int> is;
    float y{-1.0f};
    vs.insert(vs.end(), {0.0f, y, 0.0f,     0.0f, -1.0f, 0.0f,   0.5f, 0.0f});
    float dh = 2.0f / static_cast<float>(discs);
    y += dh;
    float s = 1.0f / static_cast<float>(segments);
    for(unsigned int i{1}; i <= segments; ++i)
    {
        float a = 2 * M_PI * (i - 1) / segments;
        float r = sqrtf(1.0 - y * y);
        glm::vec3 v{sinf(a) * r, y, cosf(a) * r};
        vs.insert(vs.end(), {v.x, v.y, v.z,     1.0f, 0.0f, 0.0f,   (i - 1) * s, y * 0.5f + 0.5f});
        is.insert(is.end(), {0, (i % segments) + 1, i});
    }
    for(unsigned int j{1}; j < discs - 1; ++j)
    {
        y += dh;
        float r = sqrtf(1.0 - y * y);
        for(unsigned int i{1}; i <= segments; ++i)
        {
            float a = 2 * M_PI * (i - 1) / segments;
            glm::vec3 v{sinf(a) * r, y, cosf(a) * r};
            vs.insert(vs.end(), {v.x, v.y, v.z,     1.0f, 0.0f, 0.0f,   (i - 1) * s, y * 0.5f + 0.5f});
            is.insert(is.end(), {(j - 1) * segments + i, j * segments + (i % segments) + 1, j * segments + i,
                j * segments + (i % segments) + 1, (j - 1) * segments + i, (j - 1) * segments + (i % segments) + 1});
        }
    }
    y += dh;

    vs.insert(vs.end(), {0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,   0.5f, 1.0f});
    for(unsigned int i{1}; i <= segments; ++i)
    {
        auto last = static_cast<unsigned int>((discs - 1) * segments + 1);
        auto li = last - segments;
        is.insert(is.end(), {last, li + i % segments, li + (i + 1) % segments});
    }

    return {vs, is};
}