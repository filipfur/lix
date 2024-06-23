#include "sphere.h"

#include <stdexcept>

lix::Sphere::Sphere(const lix::Sphere& other)
    : Shape{other}, _radii{other._radii}
{
}

lix::Sphere::Sphere(float radii)
    : Shape{}, _radii{radii}
{

}

lix::Sphere::~Sphere() noexcept
{

}

lix::Sphere* lix::Sphere::clone() const
{
    return new lix::Sphere(*this);
}

glm::vec3 lix::Sphere::supportPoint(const glm::vec3& dir)
{
    return translation() + glm::normalize(dir) * _radii;
}

bool lix::Sphere::intersects(Sphere& sphere)
{
    glm::vec3 delta = translation() - sphere.translation();
    float d2 = glm::dot(delta, delta);
    float srad = _radii + sphere._radii;
    float r2 = srad * srad;
    return d2 <= r2;
}

bool lix::Sphere::intersects(Polygon& /*polygon*/)
{
    throw std::runtime_error("sphere-polygon collision not implemented");
}

bool lix::Sphere::test(Shape& shape)
{
    return shape.intersects(*this);
}