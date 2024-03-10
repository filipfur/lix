#include "sphere.h"

#include <stdexcept>

impact::Sphere::Sphere(const glm::vec3& position, float radii)
    : _position{position}, _radii{radii}
{

}

impact::Sphere::~Sphere() noexcept
{

}

glm::vec3 impact::Sphere::supportPoint(const glm::vec3& dir)
{
    return _position + glm::normalize(dir) * _radii;
}

const glm::vec3& impact::Sphere::position()
{
    return _position;
}

impact::Sphere* impact::Sphere::setPosition(const glm::vec3& position)
{
    _position = position;
    if(_simplified)
    {
        _simplified->setPosition(position);
    }
    return this;
}

impact::Sphere* impact::Sphere::move(const glm::vec3& delta)
{
    _position += delta;
    if(_simplified)
    {
        _simplified->move(delta);
    }
    return this;
}

bool impact::Sphere::intersects(Sphere& sphere)
{
    glm::vec3 delta = _position - sphere._position;
    float d2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    float srad = _radii + sphere._radii;
    float r2 = srad * srad;
    return d2 <= r2;
}

bool impact::Sphere::intersects(Polygon& /*polygon*/)
{
    throw std::runtime_error("sphere-polygon collision not implemented");
}

bool impact::Sphere::test(Shape& shape)
{
    return shape.intersects(*this);
}