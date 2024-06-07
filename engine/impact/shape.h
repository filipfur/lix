#pragma once

#include <memory>
#include <functional>
#include "glm/glm.hpp"
#include <vector>

namespace lix
{
    class Shape
    {
    public:
        virtual ~Shape() noexcept;

        virtual glm::vec3 supportPoint(const glm::vec3& dir) = 0;
        virtual const glm::vec3& position() = 0;
        virtual Shape* setPosition(const glm::vec3& position) = 0;
        virtual Shape* move(const glm::vec3& delta) = 0;
        virtual bool intersects(class Sphere& sphere) = 0;
        virtual bool intersects(class Polygon& polygon) = 0;
        virtual bool test(Shape& shape) = 0;
        void setSimplified(std::shared_ptr<Shape> simplified);
        Shape* simplified();
        // starts from simplest and keeps testing more detailed version until return false
        bool doRecursive(const std::function<bool(Shape*)>& callback);

        void swapIndices(unsigned int a, unsigned int b);
        void clearIndices();

    protected:
        std::vector<unsigned int> _storedIndices;
        std::shared_ptr<Shape> _simplified{nullptr};
    };
}