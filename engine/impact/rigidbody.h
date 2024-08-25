#pragma once

#include "shape.h"

namespace lix {
struct RigidBody {
    RigidBody(std::shared_ptr<lix::Shape> shape_) : shape{shape_} {}
    virtual bool is_dynamic() = 0;
    std::shared_ptr<lix::Shape> shape;
};

struct StaticBody : public RigidBody {
    StaticBody(std::shared_ptr<lix::Shape> shape_) : RigidBody{shape_} {}
    virtual bool is_dynamic() { return false; }
};

struct DynamicBody : RigidBody {
    DynamicBody(std::shared_ptr<lix::Shape> shape_, float mass,
                const glm::mat3 &inertiaTensor)
        : RigidBody{shape_}, mass_inv{1.0f / mass},
          inertiaTensor_inv{glm::inverse(inertiaTensor)}, velocity{0.0f},
          angularVelocity{0.0f} {}
    virtual bool is_dynamic() { return true; }
    float mass_inv;
    glm::mat3 inertiaTensor_inv;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;
};
} // namespace lix