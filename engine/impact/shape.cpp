#include "shape.h"

lix::Shape::~Shape() noexcept
{
    _simplified.reset();
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

void lix::Shape::swapIndices(unsigned int a, unsigned int b)
{
    std::swap(_storedIndices[a], _storedIndices[b]);
}

void lix::Shape::clearIndices()
{
    _storedIndices.clear();
}