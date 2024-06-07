#pragma once

#include "shape.h"

namespace lix
{
class Sphere : public Shape
{
public:
    Sphere(const glm::vec3& position, float radii);

    virtual ~Sphere() noexcept;

    virtual glm::vec3 supportPoint(const glm::vec3& dir) override;
    virtual const glm::vec3& position() override;
    virtual Sphere* setPosition(const glm::vec3& position) override;
    virtual Sphere* move(const glm::vec3& delta) override;
    virtual bool intersects(Sphere& sphere) override;
    virtual bool intersects(Polygon& polygon) override;
    virtual bool test(Shape& shape) override;

private:
    glm::vec3 _position;
    float _radii;
};
}