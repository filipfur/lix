#include "aabb.h"

#include <stdexcept>
#include "primer.h"

#include "sphere.h"
#include "polygon.h"

static std::pair<glm::vec3, glm::vec3> minMaxFromPolygon(lix::Polygon& polygon)
{
    const auto& pts = polygon.points();
    std::vector<glm::vec3> rotatedPoints(pts.size());
    std::transform(pts.begin(), pts.end(), rotatedPoints.begin(), [&polygon](const glm::vec3& p) {
        return glm::vec3(glm::scale(polygon.trs()->rotationMatrix(), polygon.trs()->scale()) * glm::vec4{p, 1.0f});
    });
    return lix::extremePoints(rotatedPoints);
}

lix::AABB::AABB(const lix::AABB& other)
    : Shape{other}, _min{other._min}, _max{other._max}
{
}

lix::AABB::AABB(lix::TRS* trs, const glm::vec3& min, const glm::vec3& max)
    : Shape{trs}, _min{min}, _max{max}
{

}

lix::AABB::AABB(lix::TRS* trs, lix::Polygon* polygon)
    : Shape{trs}, _polygon{polygon}
{
    auto [min, max] = minMaxFromPolygon(*_polygon);
    _min = min;
    _max = max;
}

lix::AABB::~AABB() noexcept
{

}

lix::AABB* lix::AABB::clone() const
{
    return new lix::AABB(*this);
}

void lix::AABB::updateMinMax()
{
    if(_polygon)
    {
        if(_polygon->trs()->rotationVersionSync(_rVersion)) // rotation changed
        {
            auto [min, max] = minMaxFromPolygon(*_polygon);
            _min = min;
            _max = max;
        }
    }
}

std::vector<glm::vec3> lix::AABB::boundingBox()
{
    updateMinMax();
    /*glm::vec3 a = trs()->translation() + _min;
    glm::vec3 b = trs()->translation() + _max;
    return lix::minimumBoundingBox(a, b);*/
    return lix::minimumBoundingBox(_min, _max);
}

glm::vec3 lix::AABB::supportPoint(const glm::vec3& dir)
{
    /*glm::mat4 m = glm::scale(glm::translate(glm::mat4{1.0f}, trs()->translation()), trs()->scale());
    glm::vec3 a = glm::vec3(m * glm::vec4{_min, 1.0f});
    glm::vec3 b = glm::vec3(m * glm::vec4{_max, 1.0f});*/
    auto pts = boundingBox();
    auto index = lix::indexAlongDirection(pts, dir);
    return trs()->translation() + pts.at(index);
}

bool lix::AABB::intersects(Sphere& sphere)
{
    updateMinMax();
    glm::vec3 delta = trs()->translation() - sphere.trs()->translation();
    float d2 = glm::dot(delta, delta);
    float radii = sphere.radii() * sphere.trs()->scale().x;
    if(d2 < (radii * radii))
    {
        return true;
    }
    glm::vec3 p = sphere.trs()->translation() + glm::normalize(delta) * radii;
    glm::vec3 a = trs()->translation() + _min;
    glm::vec3 b = trs()->translation() + _max;
    return p.x > a.x && p.y > a.y && p.z > a.z
        && p.x < b.x && p.y < b.y && p.z < b.z;
}

bool lix::AABB::intersects(AABB& aabb)
{
    updateMinMax();
    glm::vec3 a0 = trs()->translation() + _min;
    glm::vec3 b0 = trs()->translation() + _max;
    //glm::vec3 c0 = (a0 + b0) * 0.5f;
    //glm::vec3 h0 = b0 - c0;

    aabb.updateMinMax();
    glm::vec3 a1 = aabb.trs()->translation() + aabb._min;
    glm::vec3 b1 = aabb.trs()->translation() + aabb._max;
    //glm::vec3 c1 = (a1 + b1) * 0.5f;
    //glm::vec3 h1 = b1 - c1;

    return a0.x <= b1.x && b0.x >= a1.x
        && a0.y <= b1.y && b0.y >= a1.y
        && a0.z <= b1.z && b0.z >= a1.z;
}

bool lix::AABB::intersects(Polygon& /*polygon*/)
{
    throw std::runtime_error("aabb-polygon collision not implemented");
}

bool lix::AABB::test(Shape& shape)
{
    return shape.intersects(*this);
}