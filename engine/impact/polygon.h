#pragma once

#include <vector>
#include <memory>

#include "shape.h"
#include "gltrs.h"


namespace impact
{
class Polygon : public impact::Shape, public lix::TRS
{
public:
    Polygon(const std::vector<glm::vec3>& points);
    virtual ~Polygon() noexcept;
    glm::vec3 supportPoint(const glm::vec3& dir) override;
    void setPoints(const std::vector<glm::vec3>& points);
    const std::vector<glm::vec3>& points();
    const std::vector<glm::vec3>& transformedPoints();
    glm::vec3 center();
    const glm::vec3& translation() = delete;
    lix::TRS* setTranslation(const glm::vec3& translation) = delete;
    lix::TRS* applyTranslation(const glm::vec3& translation) = delete;
    const glm::vec3& position() override;
    Polygon* setPosition(const glm::vec3& position) override;
    Polygon* move(const glm::vec3& delta) override;
    virtual bool intersects(Sphere& sphere) override;
    virtual bool intersects(Polygon& polygon) override;
    virtual bool test(Shape& shape) override;

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
