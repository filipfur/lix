#include "polygon.h"

#include <stdexcept>
#include <algorithm>

namespace
{
inline glm::vec3 centerOfPolygon(const std::vector<glm::vec3>& points)
{
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    float ratio = 1.0f / static_cast<float>(points.size());
    for(const auto& p : points)
    {
        center += p * ratio;
    }
    return center;
}
}

lix::Polygon::Polygon(const lix::Polygon& other)
    : lix::Shape{other}, _points{other._points}
{

}

lix::Polygon::Polygon(lix::TRS* trs, const std::vector<glm::vec3>& points)
    : Shape{trs}, _points{points}, _transformedPoints{}, _center{centerOfPolygon(points)}
{
}

lix::Polygon::~Polygon() noexcept
{
    _transformedPoints.clear();
}

lix::Polygon* lix::Polygon::clone() const
{
    return new lix::Polygon(*this);
}

glm::vec3 lix::Polygon::supportPoint(const glm::vec3& D)
{
    float maxVal = -FLT_MAX;
    int maxIndex{-1};
    updateTransformedPoints();
    for(size_t i{0}; i < _transformedPoints.size(); ++i)
    {
        //printf("checking: %f %f %f\n", _transformedPoints[i].x, _transformedPoints[i].y, _transformedPoints[i].z);
        float val = glm::dot(_transformedPoints[i], D);
        if(val > maxVal)
        {
            maxVal = val;
            maxIndex = static_cast<int>(i);
        }
    }
    assert(maxIndex != -1);
    return _transformedPoints.at(maxIndex);
}

const std::vector<glm::vec3>& lix::Polygon::points() const
{
    return _points;
}

const std::vector<glm::vec3>& lix::Polygon::transformedPoints()
{
    updateTransformedPoints();
    return _transformedPoints;
}

glm::vec3 lix::Polygon::center()
{
    return glm::vec3(trs()->modelMatrix() * glm::vec4(_center, 1.0f));
}

bool lix::Polygon::intersects(lix::Sphere& /*sphere*/)
{
    throw std::runtime_error("polygon-sphere collision not implemented");
}

bool lix::Polygon::intersects(lix::AABB& /*aabb*/)
{
    throw std::runtime_error("polygon-aabb collision not implemented");
}

bool lix::Polygon::intersects(lix::Polygon& /*polygon*/)
{
    throw std::runtime_error("polygon-polygon collision not implemented");
}

bool lix::Polygon::test(lix::Shape& /*shape*/)
{
    throw std::runtime_error("Polygon does not support simple test");
}

bool lix::Polygon::updateTransformedPoints()
{
    if(!trs()->modelVersionSync(_mVersion) || _points.size() != _transformedPoints.size())
    {
        _transformedPoints.resize(_points.size());
        const glm::mat4& m = trs()->modelMatrix();
        std::transform(_points.begin(), _points.end(),
            _transformedPoints.begin(), [&m](const glm::vec3& p) {
                return glm::vec3(m * glm::vec4(p, 1.0f));
            }
        );
        return true;
    }
    return false;
}