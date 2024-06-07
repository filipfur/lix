#include "sphere.h"

#include <stdexcept>

lix::Sphere::Sphere(const glm::vec3& position, float radii)
    : _position{position}, _radii{radii}
{

}

lix::Sphere::~Sphere() noexcept
{

}

glm::vec3 lix::Sphere::supportPoint(const glm::vec3& dir)
{
    return _position + glm::normalize(dir) * _radii;
}

const glm::vec3& lix::Sphere::position()
{
    return _position;
}

lix::Sphere* lix::Sphere::setPosition(const glm::vec3& position)
{
    _position = position;
    if(_simplified)
    {
        _simplified->setPosition(position);
    }
    return this;
}

lix::Sphere* lix::Sphere::move(const glm::vec3& delta)
{
    _position += delta;
    if(_simplified)
    {
        _simplified->move(delta);
    }
    return this;
}

bool lix::Sphere::intersects(Sphere& sphere)
{
    glm::vec3 delta = _position - sphere._position;
    float d2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
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