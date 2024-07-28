#pragma once

#include <vector>
#include <memory>

#include "shape.h"


namespace lix
{
class Polygon : public lix::Shape
{
public:
    Polygon(const Polygon& other);
    Polygon(lix::TRS* trs, const std::vector<glm::vec3>& points);
    virtual ~Polygon() noexcept;
    virtual Polygon* clone() const override;
    glm::vec3 supportPoint(const glm::vec3& dir) override;
    const std::vector<glm::vec3>& points() const;
    const std::vector<glm::vec3>& transformedPoints();
    glm::vec3 center();
    virtual bool intersects(Capsule& capsule) override;
    virtual bool intersects(Sphere& sphere) override;
    virtual bool intersects(AABB& aabb) override;
    virtual bool intersects(Polygon& polygon) override;
    virtual bool doTest(Shape& shape) override;

protected:
    bool updateTransformedPoints();

private:
    const std::vector<glm::vec3>& _points;
    std::vector<glm::vec3> _transformedPoints;
    uint32_t _mVersion{0};
    glm::vec3 _center;
};
}
