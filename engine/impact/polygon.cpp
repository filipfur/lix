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

impact::Polygon::Polygon(const std::vector<glm::vec3>& points)
    : _points{points}, _transformedPoints{}, _center{centerOfPolygon(points)},
    _transformedPointsInvalid{true}
{
}

impact::Polygon::~Polygon() noexcept
{
    _points.clear();
    _transformedPoints.clear();
}

void impact::Polygon::setPoints(const std::vector<glm::vec3>& points)
{
    _points = points;
    _transformedPointsInvalid = true;
    _center = centerOfPolygon(points);
}

glm::vec3 impact::Polygon::supportPoint(const glm::vec3& D)
{
    float maxVal = std::numeric_limits<float>::lowest();
    int maxIndex{-1};
    updateTransformedPoints();
    for(size_t i{0}; i < _transformedPoints.size(); ++i)
    {
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

const std::vector<glm::vec3>& impact::Polygon::points()
{
    return _points;
}

const std::vector<glm::vec3>& impact::Polygon::transformedPoints()
{
    updateTransformedPoints();
    return _transformedPoints;
}

glm::vec3 impact::Polygon::center()
{
    return glm::vec3(model() * glm::vec4(_center, 1.0f));
}

const glm::vec3& impact::Polygon::position()
{
    return lix::TRS::translation();
}

impact::Polygon* impact::Polygon::setPosition(const glm::vec3& position)
{
    lix::TRS::setTranslation(position);
    _transformedPointsInvalid = true;
    if(_simplified)
    {
        _simplified->setPosition(position);
    }
    return this;
}

impact::Polygon* impact::Polygon::move(const glm::vec3& delta)
{
    lix::TRS::applyTranslation(delta);
    _transformedPointsInvalid = true;
    if(_simplified)
    {
        _simplified->move(delta);
    }
    return this;
}

bool impact::Polygon::intersects(impact::Sphere& /*sphere*/)
{
    throw std::runtime_error("polygon-sphere collision not implemented");
}

bool impact::Polygon::intersects(impact::Polygon& /*polygon*/)
{
    throw std::runtime_error("polygon-polygon collision not implemented");
}

bool impact::Polygon::test(impact::Shape& /*shape*/)
{
    throw std::runtime_error("Polygon does not support simple test");
}

bool impact::Polygon::updateTransformedPoints()
{
    if(_transformedPointsInvalid)
    {
        _transformedPoints.resize(_points.size());
        const glm::mat4& m = model();
        std::transform(_points.begin(), _points.end(),
            _transformedPoints.begin(), [&m](const glm::vec3& p) {
                return glm::vec3(m * glm::vec4(p, 1.0f));
            }
        );
        _transformedPointsInvalid = false;
        return true;
    }
    return false;
}

bool impact::Polygon::updateModelMatrix()
{
    if(lix::TRS::updateModelMatrix())
    {
        _transformedPointsInvalid = true;
        return true;
    }
    return false;
}