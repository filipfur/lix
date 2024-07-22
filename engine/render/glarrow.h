#pragma once

#include "glnode.h"

namespace lix
{
    std::shared_ptr<lix::Node> arrow(lix::Color color, const glm::vec3& p, const glm::vec3& d);
}