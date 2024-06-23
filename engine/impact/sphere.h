#pragma once

#include "shape.h"

namespace lix
{
class Sphere : public Shape
{
public:
    Sphere(const Sphere& other);
    Sphere(float radii);

    virtual ~Sphere() noexcept;

    virtual Sphere* clone() const override;

    virtual glm::vec3 supportPoint(const glm::vec3& dir) override;
    virtual bool intersects(Sphere& sphere) override;
    virtual bool intersects(Polygon& polygon) override;
    virtual bool test(Shape& shape) override;

private:
    float _radii{0.0f};
};
}