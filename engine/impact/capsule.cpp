#include "capsule.h"

#include <stdexcept>

#include "sphere.h"
#include "aabb.h"

lix::Capsule::Capsule(const lix::Capsule& other)
    : Shape{other}, _a{other._a}, _b{other._b}, _radii{other._radii}
{
}

lix::Capsule::Capsule(lix::TRS* trs, const glm::vec3& a, const glm::vec3& b, float radii, bool caps)
    : Shape{trs}, _a{a}, _b{b}, _radii{radii}, _caps{caps}
{

}

lix::Capsule::~Capsule() noexcept
{

}

lix::Capsule* lix::Capsule::clone() const
{
    return new lix::Capsule(*this);
}

glm::vec3 lix::Capsule::supportPoint(const glm::vec3&)
{
    throw std::runtime_error("capsule support point not implemented");
}

bool lix::Capsule::intersects(Capsule&)
{
    throw std::runtime_error("capsule-capsule collision not implemented");
}

bool lix::Capsule::intersects(Sphere& sphere)
{
    glm::mat3 m = glm::mat3(_trs->modelMatrix());
    const glm::vec3 A = _trs->translation() + m * _a;
    const glm::vec3 B = _trs->translation() + m * _b;

    const glm::vec3& C = sphere.trs()->translation();

    const glm::vec3 AB = B - A;
    const glm::vec3 AC = C - A;

    const float len = glm::length(AB);
    assert(len > FLT_EPSILON);
    const glm::vec3 n = AB / len;

    float d = glm::dot(AC, n);

    float r1 = _radii * _trs->scale().x;
    float r2 = sphere.radii() * sphere.trs()->scale().x;
    if(_caps)
    {
        if(d < -r1 || d > (len + r1))
        {
            return false;
        }
        d = std::min(len, std::max(0.0f, d));
    }
    else
    {
        if(d < 0 || d > len)
        {
            return false;
        }
    }

    const glm::vec3 p_line = A + n * d;
    glm::vec3 delta = C - p_line;
    float d2 = glm::dot(delta, delta);
    float r = r1 + r2;
    return d2 < (r * r);
}

bool lix::Capsule::intersects(lix::AABB& aabb)
{
    return aabb.intersects(*this);
}

bool lix::Capsule::intersects(Polygon&)
{
    throw std::runtime_error("capsule-polygon collision not implemented");
}

bool lix::Capsule::doTest(Shape& shape)
{
    return shape.intersects(*this);
}