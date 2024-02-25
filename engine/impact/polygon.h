#pragma once

#include <vector>
#include <memory>

#include "shape.h"
#include "glm/glm.hpp"

namespace impact
{
class Polygon : public impact::Shape
{
public:
    Polygon(const std::vector<glm::vec3>& points);

    glm::vec3 supportPoint(const glm::vec3& dir) override;

    void setPoints(const std::vector<glm::vec3>& points);

    const std::vector<glm::vec3>& points();

    const std::vector<glm::vec3>& transformedPoints();

    glm::vec3 center() override;

protected:
    bool updateTransformedPoints();

    virtual bool updateModelMatrix() override;

private:
    std::vector<glm::vec3> _points;
    std::vector<glm::vec3> _transformedPoints;
    glm::vec3 _center;
    bool _transformedPointsInvalid;
};
}
