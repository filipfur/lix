#pragma once

#include <vector>
#include <list>

#include "halfedge.h"

#include "glm/glm.hpp"

namespace lix
{
    class Convex_Hull
    {
    public:
        Convex_Hull(const std::vector<glm::vec3>& points);
        ~Convex_Hull() noexcept;

        void add_point(const glm::vec3& p);

        void mesh_data(std::vector<float>& points, std::vector<unsigned int>& indices);

        std::list<Face>::iterator begin()
        {
            return _faces.begin();
        }

        std::list<Face>::iterator end()
        {
            return _faces.end();
        }

    private:
        void connect(Half_Edge* self,
            Half_Edge* next,
            Half_Edge* opposite,
            Face* face);

        std::list<Face> _faces;
    };
}