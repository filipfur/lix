#pragma once

#include "shape.h"

namespace lix {
class AABB : public Shape {
  public:
    AABB(const AABB &other);
    AABB(lix::TRS *trs, const glm::vec3 &min, const glm::vec3 &max);
    AABB(lix::TRS *trs, class Polygon *polygon);

    virtual ~AABB() noexcept;

    virtual AABB *clone() const override;

    virtual glm::vec3 supportPoint(const glm::vec3 &dir) override;
    virtual bool intersects(Capsule &capsule) override;
    virtual bool intersects(Sphere &sphere) override;
    virtual bool intersects(AABB &aabb) override;
    virtual bool intersects(Polygon &polygon) override;
    virtual bool doTest(Shape &shape) override;

    std::vector<glm::vec3> boundingBox();

    const glm::vec3 &min() const { return _min; }

    const glm::vec3 &max() const { return _max; }

  private:
    void updateMinMax();

    glm::vec3 _min;
    glm::vec3 _max;
    class Polygon *_polygon{nullptr};
    uint32_t _rVersion{0};
};
} // namespace lix