#include "shape.h"

impact::Shape::~Shape() noexcept
{
    _simplified.reset();
}

void impact::Shape::setSimplified(std::shared_ptr<impact::Shape> simplified)
{
    _simplified = simplified;
}

impact::Shape* impact::Shape::simplified()
{
    return _simplified.get();
}

bool impact::Shape::doRecursive(const std::function<bool(Shape*)>& callback)
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