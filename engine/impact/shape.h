#pragma once

#include "glm/glm.hpp"
#include "gltrs.h"
#include <functional>
#include <memory>
#include <vector>

namespace lix {
class Shape {
  public:
    Shape(lix::TRS *trs);
    Shape(const Shape &other);
    Shape(Shape &&other) = delete;

    Shape &operator=(const Shape &other) = delete;
    Shape &operator=(Shape &&other) = delete;

    virtual Shape *clone() const = 0;

    virtual ~Shape() noexcept;

    virtual glm::vec3 supportPoint(const glm::vec3 &dir) = 0;

    virtual bool intersects(class Capsule &sphere) = 0;
    virtual bool intersects(class Sphere &sphere) = 0;
    virtual bool intersects(class AABB &aabb) = 0;
    virtual bool intersects(class Polygon &polygon) = 0;
    bool test(Shape &shape);
    virtual bool doTest(Shape &shape) = 0;
    void setSimplified(std::shared_ptr<Shape> simplified);
    Shape *simplified();
    // starts from simplest and keeps testing more detailed version until return
    // false
    bool doRecursive(const std::function<bool(Shape *)> &callback);

    void addPositive(std::shared_ptr<Shape> positive) {
        _positives.push_back(positive);
    }

    void addNegative(std::shared_ptr<Shape> negative) {
        _negatives.push_back(negative);
    }

    lix::TRS *trs() const { return _trs; }

    void setTRS(lix::TRS *trs) {
        _trs = trs;
        if (_simplified) {
            _simplified->setTRS(trs);
        }
    }

  protected:
    lix::TRS *_trs{nullptr};
    std::shared_ptr<Shape> _simplified{nullptr};
    std::vector<std::shared_ptr<Shape>> _positives;
    std::vector<std::shared_ptr<Shape>> _negatives;
};
} // namespace lix