#pragma once

#include <vector>
#include <list>
#include <optional>

#include "halfedge.h"

#include "glm/glm.hpp"

namespace lix
{
    class ConvexHull
    {
    public:
        //ConvexHull(const std::vector<lix::Vertex>& points);
        ConvexHull(const std::vector<glm::vec3>& points);
        ~ConvexHull() noexcept;

        bool addPoint(const glm::vec3& p, lix::Face* face=nullptr, std::list<Face*>* Q=nullptr);
        void addPoints(std::vector<glm::vec3>& P);

        std::pair<std::vector<glm::vec3>, std::vector<unsigned int>> meshData() const;

        std::vector<glm::vec3> points() const;

        std::list<Face>::iterator begin()
        {
            return _faces.begin();
        }

        std::list<Face>::iterator end()
        {
            return _faces.end();
        }

        std::list<Face>::const_iterator begin() const
        {
            return _faces.begin();
        }

        std::list<Face>::const_iterator end() const
        {
            return _faces.end();
        }

        std::optional<glm::vec3> rayIntersect(const glm::vec3& p, const glm::vec3& d) const;

    private:
        void connect(Half_Edge* self,
            Half_Edge* next,
            Half_Edge* opposite,
            Face* face);

        std::list<Face> _faces;
    };
}