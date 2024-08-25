#include "shape.h"

lix::Shape::Shape(lix::TRS *trs) : _trs{trs} {}

lix::Shape::Shape(const lix::Shape &other)
    : _trs{other._trs},
      _simplified{other._simplified ? other._simplified->clone() : nullptr} {}

lix::Shape::~Shape() noexcept { _simplified.reset(); }

void lix::Shape::setSimplified(std::shared_ptr<lix::Shape> simplified) {
    _simplified = simplified;
}

lix::Shape *lix::Shape::simplified() { return _simplified.get(); }

bool lix::Shape::test(lix::Shape &shape) {
    bool anyPositive = doTest(shape);
    if (!anyPositive) {
        for (auto &positive : _positives) {
            if (positive->test(shape)) {
                anyPositive = true;
                break;
            }
        }
    }
    if (!anyPositive) {
        return false;
    }
    bool anyNegative{false};
    for (auto &negative : _negatives) {
        if (negative->test(shape)) // should be overlaps
        {
            anyNegative = true;
            break;
        }
    }
    return !anyNegative;
}

bool lix::Shape::doRecursive(const std::function<bool(Shape *)> &callback) {
    static bool doTest;
    if (_simplified) {
        _simplified->doRecursive(callback);
    } else {
        doTest = true;
    }
    doTest = doTest && callback(this);
    return doTest;
}