#pragma once

#include "shape.h"

namespace lix {
class Sphere : public Shape {
  public:
    Sphere(const Sphere &other);
    Sphere(lix::TRS *trs, float radii);

    virtual ~Sphere() noexcept;

    virtual Sphere *clone() const override;

    virtual glm::vec3 supportPoint(const glm::vec3 &dir) override;
    virtual bool intersects(Capsule &capsule) override;
    virtual bool intersects(Sphere &sphere) override;
    virtual bool intersects(AABB &aabb) override;
    virtual bool intersects(Polygon &polygon) override;
    virtual bool doTest(Shape &shape) override;

    float radii() const { return _radii; }

  private:
    float _radii{0.0f};
};
} // namespace lix