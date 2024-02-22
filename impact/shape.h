#pragma once

#include "glm.hpp"
#include "gltrs.h"

namespace impact
{
    class Shape : public lix::TRS
    {
        public:
        virtual glm::vec3 supportPoint(const glm::vec3& dir) = 0;
        virtual glm::vec3 center() = 0;
    };
}