#include "shape.h"

lix::Shape::Shape(lix::TRS* trs)
    : _trs{trs}
{

}

lix::Shape::Shape(const lix::Shape& other)
    : _trs{other._trs}, _simplified{other._simplified ? other._simplified->clone() : nullptr}
{

}

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