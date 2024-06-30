#pragma once

#include "collision.h"

#include <unordered_map>
#include "glnode.h"

namespace lix
{
    struct RigidBody
    {
        unsigned int id;
        std::shared_ptr<lix::Shape> shape;
        bool collides;
        bool dynamic;
        float mass_inv;
        glm::vec3 velocity;
        glm::vec3 angularVelocity;
        glm::mat3 I_inv;
    };

    class CollisionSystem
    {
    public:
        static std::shared_ptr<lix::RigidBody> createRigidBody(std::shared_ptr<lix::Shape> shape, bool dynamic, float mass, float sideLength);
        
        void tick(std::vector<std::shared_ptr<lix::RigidBody>>& rigidBodies, float dt);

    private:
        void resolveCollisions(std::vector<std::shared_ptr<lix::RigidBody>>& rigidBodies);

        std::unordered_map<unsigned int, std::unordered_map<unsigned int, lix::Collision>> _collisions;
    };
}