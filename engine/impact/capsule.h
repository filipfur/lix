#pragma once

#include "shape.h"

namespace lix
{
class Capsule : public Shape
{
public:
    Capsule(const Capsule& other);
    Capsule(lix::TRS* trs, const glm::vec3& a, const glm::vec3& b, float radii, bool caps=true);

    virtual ~Capsule() noexcept;

    virtual Capsule* clone() const override;

    virtual glm::vec3 supportPoint(const glm::vec3& dir) override;
    virtual bool intersects(Capsule& capsule) override;
    virtual bool intersects(Sphere& sphere) override;
    virtual bool intersects(AABB& aabb) override;
    virtual bool intersects(Polygon& polygon) override;
    virtual bool doTest(Shape& shape) override;

    const glm::vec3& a() const
    {
        return _a;
    }

    const glm::vec3& b() const
    {
        return _b;
    }

    bool caps() const
    {
        return _caps;
    }

    float radii() const
    {
        return _radii;
    }

private:
    glm::vec3 _a;
    glm::vec3 _b;
    float _radii{1.0f};
    bool _caps{true};
};
}