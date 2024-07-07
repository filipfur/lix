#include "physicsengine.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include "collision.h"
#include "gltimer.h"

inline static float impulse(lix::DynamicBody& dynamicBody, const lix::Collision& collision, float restitution)
{
    const glm::vec3 r = collision.contactPoint - dynamicBody.shape->trs()->translation();
    const glm::vec3 vrel = dynamicBody.velocity + glm::cross(dynamicBody.angularVelocity, r);
    
    float vrel_n = glm::dot(vrel, collision.normal);

    glm::vec3 r_cross_n = glm::cross(r, collision.normal);
    float denominator = dynamicBody.mass_inv + glm::dot(collision.normal, glm::cross(dynamicBody.inertiaTensor_inv * r_cross_n, r));

    float J = -(1.0f + restitution) * vrel_n / denominator;

    return J;
}

inline static void updateDerivatives(lix::DynamicBody& rigidBody, float dt)
{
    rigidBody.velocity.y = std::max(-8.0f, rigidBody.velocity.y - 1.0f * dt);

    float dampingFactor = exp(-0.5f * dt);
    rigidBody.angularVelocity *= dampingFactor;
    glm::quat deltaRotation = glm::quat(0.0f, rigidBody.angularVelocity * dt); // Small rotation step
}

inline static void forwardBody(lix::DynamicBody& rigidBody, float dt)
{
    rigidBody.shape->trs()->applyTranslation(rigidBody.velocity * dt);
    glm::quat deltaRotation = glm::quat(0.0f, rigidBody.angularVelocity * dt);
    rigidBody.shape->trs()->setRotation(glm::normalize(rigidBody.shape->trs()->rotation() + 0.5f * deltaRotation * rigidBody.shape->trs()->rotation()));
    //rigidBody.shape->trs()->setRotation(glm::slerp(rigidBody.shape->trs()->rotation(), rigidBody.shape->trs()->rotation() + glm::quat(0.0f, rigidBody.angularVelocity * dt), 0.5f));
}

inline static void backwardBody(lix::DynamicBody& rigidBody, float dt)
{
    rigidBody.shape->trs()->applyTranslation(-rigidBody.velocity * dt);
}

inline static bool broadPhaseCollision(lix::RigidBody& bodyA, lix::RigidBody& bodyB)
{
    return bodyA.shape->simplified()->test(*bodyB.shape->simplified());
}

static inline bool narrowPhaseCollision(lix::RigidBody& bodyA, lix::RigidBody& bodyB,
    std::vector<lix::Vertex>& simplex,
    const glm::vec3& D,
    lix::Collision* collision=nullptr)
{
    simplex.clear();
    //std::vector<lix::Vertex> simplex;
    //glm::vec3 D = glm::normalize(bodyB.shape->trs()->translation() - bodyA.shape->trs()->translation());
    return lix::gjk(*bodyA.shape, *bodyB.shape, simplex, D, collision);
}

inline static float backtrackCollision(lix::DynamicBody& dynamicBody,
    lix::RigidBody& staticBody,
    float dt,
    std::vector<lix::Vertex>& simplex, const glm::vec3& D, lix::Collision& collision)
{
    //return 0.0f;
    float t = dt;
    bool backtrack{true};
    float rewindedTime{0.0f};
    for(size_t i{0}; i < 2; ++i)
    {
        t *= 0.5f;
        if(backtrack)
        {
            backwardBody(dynamicBody, t);
            rewindedTime += t;
        }
        else
        {
            forwardBody(dynamicBody, t);
            rewindedTime -= t;
        }
        backtrack = narrowPhaseCollision(dynamicBody, staticBody, simplex, D, &collision);
    }
    unsigned short counter{0};
    while(!backtrack)
    {
        forwardBody(dynamicBody, t);
        rewindedTime -= t;
        backtrack = narrowPhaseCollision(dynamicBody, staticBody, simplex, D, &collision);
        if(++counter > 2)
        {
            printf("Error: failed to backtrack collision\n");
            return -1;
        }
    }
    return rewindedTime;
}

void lix::PhysicsEngine::step(std::vector<lix::DynamicBody>& dynamicBodies,
    std::vector<lix::StaticBody>& staticBodies,
    float dt)
{
    for(auto& dynamicBody : dynamicBodies)
    {
        updateDerivatives(dynamicBody, dt);
        forwardBody(dynamicBody, dt); // fast forward
    }
    std::vector<std::pair<lix::DynamicBody&, lix::RigidBody&>> broadPhaseCollisions;
    glm::vec3 D = glm::ballRand(1.0f);
    std::vector<lix::Vertex> simplex;
    lix::Collision collision;
    for(auto& dynamicBody : dynamicBodies)
    {
        for(auto& staticBody : staticBodies)
        {
            if(broadPhaseCollision(dynamicBody, staticBody) && narrowPhaseCollision(dynamicBody, staticBody, simplex, D))
            {
                broadPhaseCollisions.emplace_back(dynamicBody, staticBody);
                break;
            }
        }
    }
    for(const auto& entry : broadPhaseCollisions)
    {
        auto& dynamicBody = entry.first;
        auto& staticBody = entry.second;

        float rewindedTime = backtrackCollision(dynamicBody, staticBody, dt, simplex, D, collision);

        if(rewindedTime < 0)
        {
            continue;
        }

        // bodies are colliding but with small margin
        if(!lix::epa(*dynamicBody.shape, *staticBody.shape, simplex, &collision))
        {
            printf("EPA fail\n");
        }
        /*if(bodyA.dynamic || bodyB.dynamic)
        {
            _collisions[minId][maxId] = std::move(collision);
        }*/
        dynamicBody.velocity = glm::reflect(dynamicBody.velocity, collision.normal);
        dynamicBody.shape->trs()->applyTranslation(collision.normal * collision.penetrationDepth);
        
        float J = impulse(dynamicBody, collision, 0.5f);
        assert(J != 0);
        const glm::vec3 r = collision.contactPoint - dynamicBody.shape->trs()->translation();
        dynamicBody.angularVelocity += glm::cross(r, collision.normal * J) * dynamicBody.inertiaTensor_inv * 0.5f;

        if(rewindedTime > 0)
        {
            forwardBody(dynamicBody, rewindedTime);
        }
    }
}

inline static glm::mat3 computeInertiaTensor(float mass, float sideLength) {
    float inertia = mass * sideLength * sideLength;
    return {
        0.66f * inertia, -0.25f * inertia, -0.25f * inertia,
        -0.25f * inertia, 0.66f * inertia, -0.25f * inertia,
        -0.25f * inertia, -0.25f * inertia, 0.66f * inertia
    };
}

lix::StaticBody lix::PhysicsEngine::createStaticBody(
    std::shared_ptr<lix::Shape> shape
)
{
    return{
        shape
    };
}

lix::DynamicBody lix::PhysicsEngine::createDynamicBody(
    std::shared_ptr<lix::Shape> shape,
    float mass,
    float sideLength
)
{
    return {
        shape,
        mass,
        computeInertiaTensor(mass, sideLength)
    };
}