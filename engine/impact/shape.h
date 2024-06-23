#pragma once

#include <memory>
#include <functional>
#include "glm/glm.hpp"
#include <vector>
#include "gltrs.h"

namespace lix
{
    class Shape : public lix::TRS
    {
    public:
        Shape();
        Shape(const Shape& other);
        Shape(Shape&& other) = delete;

        Shape& operator=(const Shape& other) = delete;
        Shape& operator=(Shape&& other) = delete;

        virtual Shape* clone() const = 0;

        virtual ~Shape() noexcept;

        virtual glm::vec3 supportPoint(const glm::vec3& dir) = 0;

        virtual lix::Shape* setTranslation(const glm::vec3& translation) override;
        virtual lix::Shape* applyTranslation(const glm::vec3& translation) override;

        virtual lix::Shape* setRotation(const glm::quat& rotation) override;
        virtual lix::Shape* applyRotation(const glm::quat& rotation) override;

        virtual lix::Shape* setScale(const glm::vec3& scale) override;
        virtual lix::Shape* applyScale(const glm::vec3& scale) override;

        virtual bool intersects(class Sphere& sphere) = 0;
        virtual bool intersects(class Polygon& polygon) = 0;
        virtual bool test(Shape& shape) = 0;
        void setSimplified(std::shared_ptr<Shape> simplified);
        Shape* simplified();
        // starts from simplest and keeps testing more detailed version until return false
        bool doRecursive(const std::function<bool(Shape*)>& callback);

    protected:
        std::shared_ptr<Shape> _simplified{nullptr};
    };
}