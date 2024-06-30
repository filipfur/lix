#include "collisionsystem.h"

#include "response.h"

#include "glm/gtc/random.hpp"

inline static glm::mat3 computeInertiaTensor(float mass, float sideLength) {
    float inertia = mass * sideLength * sideLength;
    return {
        0.66f * inertia, -0.25f * inertia, -0.25f * inertia,
        -0.25f * inertia, 0.66f * inertia, -0.25f * inertia,
        -0.25f * inertia, -0.25f * inertia, 0.66f * inertia
    };
}

void collisionResponse(lix::RigidBody& rigidBody, const glm::vec3& normal, float penetration)
{
    rigidBody.velocity = glm::reflect(rigidBody.velocity, normal);
    rigidBody.shape->trs()->applyTranslation(normal * penetration);
}

std::shared_ptr<lix::RigidBody> lix::CollisionSystem::createRigidBody(
    std::shared_ptr<lix::Shape> shape,
    bool dynamic,
    float mass,
    float sideLength
)
{
    static unsigned int nextId{0};
    float aMass = dynamic ? mass : FLT_MAX;
    return std::shared_ptr<lix::RigidBody>{
        new lix::RigidBody{
            nextId++,
            shape,
            false,
            dynamic,
            1.0f / aMass,
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            glm::inverse(computeInertiaTensor(mass, sideLength))
        }
    };
}
void lix::CollisionSystem::tick(std::vector<std::shared_ptr<lix::RigidBody>>& rigidBodies, float dt)
{
    resolveCollisions(rigidBodies);

    float deltaTime = dt;
    while(deltaTime > 0)
    {
        float ddt = glm::min(deltaTime, 0.01f);
        deltaTime -= ddt;
        for(const auto& rigidBody : rigidBodies)
        {
            if(rigidBody->dynamic)
            {
                rigidBody->velocity.y = std::max(-8.0f, rigidBody->velocity.y - 2.0f * ddt);
                rigidBody->shape->trs()->applyTranslation(rigidBody->velocity * ddt);
                float dampingFactor = exp(-0.5f * ddt);
                rigidBody->angularVelocity *= dampingFactor;
                glm::quat deltaRotation = glm::quat(0.0f, rigidBody->angularVelocity * ddt); // Small rotation step
                rigidBody->shape->trs()->setRotation(glm::normalize(rigidBody->shape->trs()->rotation() + 0.5f * deltaRotation * rigidBody->shape->trs()->rotation()));
            }
            rigidBody->shape->trs()->setTranslation(rigidBody->shape->trs()->translation());
            rigidBody->collides = false;
        }

        for(const auto& bodyA : rigidBodies)
        {
            for(const auto& bodyB : rigidBodies)
            {
                if(bodyA->id >= bodyB->id)
                {
                    continue;
                }
                unsigned int minId = bodyA->id;
                unsigned int maxId = bodyB->id;
                auto minIt = _collisions.find(minId);
                if(minIt != _collisions.end())
                {
                    auto maxIt = minIt->second.find(maxId);
                    if(maxIt != minIt->second.end())
                    {
                        continue; // already colliding
                    }
                }
                if(bodyA->shape->simplified() && bodyB->shape->simplified())
                {
                    if(!bodyA->shape->simplified()->test(*bodyB->shape->simplified()))
                    {
                        //printf("simply not a collision %d %d\n", minId, maxId);
                        continue;
                    }
                }
                std::vector<lix::Vertex> simplex;
                glm::vec3 D = glm::ballRand(1.0f);
                lix::Collision collision;
                if(lix::gjk(*bodyA->shape, *bodyB->shape, simplex, D, &collision))
                {
                    if(!lix::epa(*bodyA->shape, *bodyB->shape, simplex, &collision))
                    {
                        printf("EPA fail\n");
                    }
                    if(bodyA->dynamic || bodyB->dynamic)
                    {
                        _collisions[minId][maxId] = std::move(collision);
                    }
                    bodyA->collides = true;
                    bodyB->collides = true;
                }
            }
        }
    }
}

void lix::CollisionSystem::resolveCollisions(std::vector<std::shared_ptr<lix::RigidBody>>& rigidBodies)
{
    for(const auto& cr1 : _collisions)
    {
        for(const auto& cr2 : cr1.second)
        {
            std::shared_ptr<RigidBody> r1 =  rigidBodies.at(cr1.first);
            std::shared_ptr<RigidBody> r2 =  rigidBodies.at(cr2.first);
            if(r1->dynamic)
            {
                collisionResponse(*r1, cr2.second.collisionNormal, r2->dynamic
                    ? (cr2.second.penetrationDepth * 0.5f)
                    : cr2.second.penetrationDepth);
            }
            if(r2->dynamic)
            {
                collisionResponse(*r2, -cr2.second.collisionNormal, r1->dynamic
                    ? (cr2.second.penetrationDepth * 0.5f)
                    : cr2.second.penetrationDepth);
            }
            lix::impulse(0.5f, 1.0f, 1.0f, r1->I_inv, r2->I_inv,
                cr2.second.contactPoint - r1->shape->trs()->translation(),
                cr2.second.contactPoint - r2->shape->trs()->translation(),
                cr2.second.collisionNormal, 
                r1->velocity, r2->velocity,
                r1->angularVelocity, r2->angularVelocity);
        }
    }
    _collisions.clear();
}