#pragma once

#include <memory>

#include "rigidbody.h"
#include "collision.h"
#include "halfedge.h"

namespace lix
{
    struct CharacterController
    {
        CharacterController(std::shared_ptr<lix::DynamicBody> dynamicBody_) : dynamicBody{dynamicBody_}
        {

        }

        void setMaxWalkSpeed(float maxWalkSpeed)
        {
            this->maxWalkSpeed = maxWalkSpeed;
        }

        void setMaxJumpSpeed(float maxJumpSpeed)
        {
            this->maxJumpSpeed = maxJumpSpeed;
        }

        bool onGround()
        {
            return platform != nullptr;
        }

        void jump()
        {
            if(onGround())
            {
                deltaControl.y = 1.0f;
                platform.reset();
            }
        }

        void rotate(float r)
        {
            yaw += r;
        }

        void stopJump()
        {
            deltaControl.y = 0.0f;
        }

        void moveForward() { deltaControl.z = -1.0f; }
        void moveBackward() { deltaControl.z = 1.0f; }
        void moveLeft() { deltaControl.x = -1.0f; }
        void moveRight() { deltaControl.x = 1.0f; }

        void stopForward() { deltaControl.z = 0.0f; }
        void stopBackward() { deltaControl.z = 0.0f; }
        void stopLeft() { deltaControl.x = 0.0f; }
        void stopRight() { deltaControl.x = 0.0f; }

        enum class MovementState
        {
            IDLE,
            FORWARD,
            LEFT,
            RIGHT,
            JUMPING,
            FALLING
        };

        void updateMovementState()
        {
            auto& v = dynamicBody->velocity;
            if(v.y > lix::EPSILON)
            {
                movementState = MovementState::JUMPING;
            }
            else if(v.y < -lix::EPSILON)
            {
                movementState = MovementState::FALLING;
            }
            else if(v.x * v.x + v.z * v.z > 0.01f)
            {
                if(deltaControl.z * deltaControl.z > 0.1f)
                {
                    movementState = MovementState::FORWARD;
                }
                else if(deltaControl.x > 0)
                {
                    movementState = MovementState::LEFT;
                }
                else if(deltaControl.x < 0)
                {
                    movementState = MovementState::RIGHT;
                }
            }
            else
            {
                movementState = MovementState::IDLE;
            }
        }

        float walkSpeed{0.0f};
        float maxWalkSpeed{4.0f};
        float walkAcceleration{30.0f};
        float maxJumpSpeed{5.0f};
        std::shared_ptr<lix::DynamicBody> dynamicBody;
        std::shared_ptr<lix::StaticBody> platform;
        glm::vec3 deltaControl;
        glm::vec3 heading;
        float yaw{0};
        std::list<lix::Face>::const_iterator closestFace;
        MovementState movementState{MovementState::IDLE};
        int floorCrumbling{0};
    };
}