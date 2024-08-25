#pragma once

#include "rigidbody.h"
#include <glm/glm.hpp>
#include <vector>

namespace lix {
namespace PhysicsEngine {
void step(std::vector<lix::DynamicBody> &dynamicBodies,
          std::vector<lix::StaticBody> &staticBodies, float dt);

lix::StaticBody createStaticBody(std::shared_ptr<lix::Shape> shape);

lix::DynamicBody createDynamicBody(std::shared_ptr<lix::Shape> shape,
                                   float mass, float sideLength);
} // namespace PhysicsEngine
} // namespace lix