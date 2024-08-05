#pragma once

#include "glm/glm.hpp"
#include "glnode.h"
#include "gluniformbuffer.h"


class Camera : public lix::Node
{
public:
    Camera(const glm::mat4& projection, const glm::vec3& position)
        : _block{projection, glm::mat4{1.0f}, position, 0.0f}
    {
        setTranslation(position);
    }

    void setTarget(const glm::vec3& target)
    {
        _target = target;
        _targetInvalid = true;
    }

    void setupUBO(const std::vector<lix::Element*>& shaders)
    {
        _ubo.reset(new lix::UniformBuffer(
            sizeof(CameraBlock), (void*)&_block, "CameraBlock", 0, GL_STATIC_DRAW
        ));
        _ubo->bindShaders(shaders);
        _ubo->unbind();
    }

    void refresh(float dt)
    {
        bool positionUpdated{false};
        if(updateModelMatrix() || updateGlobalMatrix())
        {
            const glm::mat4& glob = globalMatrix();
            _targetPosition = glm::vec3(glob[3]);
            positionUpdated = true;
        }
        if(positionUpdated || _targetInvalid)
        {
            _block.view = glm::lookAt(_block.position, _target, Camera::UP);
            _targetInvalid = false;
            positionUpdated = true;
        }

        glm::vec3 d{_block.position - _targetPosition};
        if(glm::length(d) > 0)
        {
            _block.position = glm::mix(_block.position, _targetPosition, 4.0f * dt);
            positionUpdated = true;
        }

        if(_viewInvalid || positionUpdated)
        {
            _ubo->bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4) + sizeof(glm::vec3)); // view + position
            _viewInvalid = false;
        }
    }

    const glm::vec3& position() const
    {
        return _block.position;
    }

    glm::vec3 forward()
    {
        return glm::vec3{_block.view[0][0], _block.view[0][1], _block.view[0][2]};
    }

    glm::vec3 up()
    {
        return glm::vec3{_block.view[1][0], _block.view[1][1], _block.view[1][2]};
    }

    glm::vec3 right()
    {
        return glm::vec3{_block.view[2][0], _block.view[2][1], _block.view[2][2]};
    }

static constexpr glm::vec3 UP{0.0f, 1.0f, 0.0f};

private:
    struct CameraBlock {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec3 position;
        float padding;
    } _block;

    bool _viewInvalid{true};

    std::shared_ptr<lix::UniformBuffer> _ubo;

    glm::vec3 _targetPosition{0.0f, 0.0f, 0.0f};

    glm::vec3 _target{0.0f, 0.0f, 0.0f};
    bool _targetInvalid{false};
};