#include "shape.h"

lix::Shape::Shape()
    : lix::TRS{}
{

}

lix::Shape::Shape(const lix::Shape& other)
    : lix::TRS{other}, _simplified{other._simplified ? other._simplified->clone() : nullptr}
{

}

lix::Shape::~Shape() noexcept
{
    _simplified.reset();
}

lix::Shape* lix::Shape::setTranslation(const glm::vec3& translation)
{
    lix::TRS::setTranslation(translation);
    if(_simplified)
    {
        _simplified->setTranslation(translation);
    }
    return this;
}
lix::Shape* lix::Shape::applyTranslation(const glm::vec3& translation)
{
    lix::TRS::applyTranslation(translation);
    if(_simplified)
    {
        _simplified->applyTranslation(translation);
    }
    return this;
}

lix::Shape* lix::Shape::setRotation(const glm::quat& rotation)
{
    lix::TRS::setRotation(rotation);
    if(_simplified)
    {
        _simplified->setRotation(rotation);
    }
    return this;
}
lix::Shape* lix::Shape::applyRotation(const glm::quat& rotation)
{
    lix::TRS::applyRotation(rotation);
    if(_simplified)
    {
        _simplified->applyRotation(rotation);
    }
    return this;
}

lix::Shape* lix::Shape::setScale(const glm::vec3& scale)
{
    lix::TRS::setScale(scale);
    if(_simplified)
    {
        _simplified->setScale(scale);
    }
    return this;
}
lix::Shape* lix::Shape::applyScale(const glm::vec3& scale)
{
    lix::TRS::applyScale(scale);
    if(_simplified)
    {
        _simplified->applyScale(scale);
    }
    return this;
}

void lix::Shape::setSimplified(std::shared_ptr<lix::Shape> simplified)
{
    _simplified = simplified;
}

lix::Shape* lix::Shape::simplified()
{
    return _simplified.get();
}

bool lix::Shape::doRecursive(const std::function<bool(Shape*)>& callback)
{
    static bool doTest;
    if(_simplified)
    {
        _simplified->doRecursive(callback);
    }
    else
    {
        doTest = true;
    }
    doTest = doTest && callback(this);
    return doTest;
}