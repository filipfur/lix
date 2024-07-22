#include "gleditor.h"

#include <iostream>

lix::Editor::Editor(const glm::mat4& perspective, const glm::vec2& resolution_, float cameraDistance) : 
    _camera{perspective, glm::vec3{8.0f, 8.0f, 8.0f}}, _resolution{resolution_}, _cameraDistance{cameraDistance}
{
    
}

void lix::Editor::onMouseDown(lix::KeySym key, lix::KeyMod /*mod*/)
{
    if(_subjectNode && _editMode != EDIT_NONE)
    {
        onSubjectTransformed(_subjectNode, static_cast<lix::Editor::Transformation>(_editMode - 1));
    }
    _editMode = EDIT_NONE;
    _translationMode = TRANS_NONE;
    switch(key)
    {
    case SDL_BUTTON_LEFT:
        _lmb = true;
        break;
    }
}

void lix::Editor::onMouseUp(lix::KeySym key, lix::KeyMod /*mod*/)
{
    switch(key)
    {
    case SDL_BUTTON_LEFT:
        _lmb = false;
        break;
    }
}

void lix::Editor::onMouseMove(float x, float y, float /*xrel*/, float /*yrel*/)
{
    static auto prevDragPos = glm::vec2{x, y} / _resolution;
    auto dragPos = glm::vec2{x, y} / _resolution;
    auto d_xy = prevDragPos - dragPos;
    d_xy *= 10.0f;
    //glm::vec3 eye = glm::vec3{0.0f} - _camera.position();
    glm::vec3 up = _camera.up();
    glm::vec3 right = _camera.right();

    //std::cout << "up=" << up.x << ", " << up.y << ", " << up.z << std::endl;
    //std::cout << "right=" << right.x << ", " << right.y << ", " << right.z << std::endl;

    //glm::vec3 right = glm::cross(eye, Camera::UP);
    //glm::vec3 forward = -glm::cross(right, Camera::UP);

    glm::vec3 delta = d_xy.x * right + d_xy.y * up;

    if(_subjectNode)
    {
        switch(_translationMode)
        {
        case TRANS_X:
            delta = glm::vec3{1.0f, 0.0f, 0.0f} * glm::dot(glm::vec3{1.0f, 0.0f, 0.0f}, delta);
            break;
        case TRANS_Y:
            delta = glm::vec3{0.0f, 1.0f, 0.0f} * glm::dot(glm::vec3{0.0f, 1.0f, 0.0f}, delta);
            break;
        case TRANS_Z:
            delta = glm::vec3{0.0f, 0.0f, 1.0f} * glm::dot(glm::vec3{0.0f, 0.0f, 1.0f}, delta);
            break;
        default:
            break;
        }
    }

    switch(_editMode)
    {
    case EDIT_TRANS:
        if(_subjectNode) { _subjectNode->setTranslation(_subjectNode->translation() + delta); }
        break;
    default:
        if(_lmb)
        {
            _cameraYaw -= d_xy.x;
            _cameraPitch -= d_xy.y;
        }
        break;
    }
    prevDragPos = dragPos;
}

void lix::Editor::onMouseWheel(float x, float y)
{
    auto mod = SDL_GetModState();

    static glm::vec2 prevDelta{x, y};
    glm::vec2 delta{x, y};

    auto d_xy = prevDelta - delta;
    d_xy *= 0.1f;

    _cameraDistance += d_xy.y;

    return; /// !!!

    if(mod & KMOD_SHIFT)
    {
        glm::vec3 rotatedVector = glm::angleAxis(_cameraYaw, glm::vec3{0.0f, 1.0f, 0.0f})
            * glm::vec3{-d_xy.y, 0.0f, d_xy.x};
        _centerPoint += rotatedVector;
    }
    else
    {
        _cameraYaw -= d_xy.x;
        _cameraPitch -= d_xy.y;
    }
}

void lix::Editor::onKeyDown(lix::KeySym key, lix::KeyMod /*mod*/)
{
    switch(key)
    {
    case SDLK_g:
        if(_editMode == EDIT_NONE)
        {
            _prevPos = _subjectNode->translation();
            setEditMode(EDIT_TRANS);
        }
        break;
    case SDLK_r:
        setEditMode(EDIT_ROT);
        break;
    case SDLK_x:
        setTranslationMode(TRANS_X);
        break;
    case SDLK_y:
        setTranslationMode(TRANS_Y);
        break;
    case SDLK_z:
        setTranslationMode(TRANS_Z);
        break;
    case SDLK_ESCAPE:
        if(_editMode != EDIT_NONE)
        {
            if(_editMode == EDIT_TRANS)
            {
                _subjectNode->setTranslation(_prevPos);
            }
            _editMode = EDIT_NONE;
            _translationMode = TRANS_NONE;
        }
        else
        {
            exit(0);
        }
        break;
    default:
        break;
    }
}

void lix::Editor::onKeyUp(lix::KeySym /*key*/, lix::KeyMod /*mod*/)
{

}

void lix::Editor::refresh(float dt)
{
    _camera.setTranslation(_centerPoint + glm::vec3{
        cosf(_cameraYaw) * cosf(_cameraPitch),
        sinf(_cameraPitch),
        sinf(_cameraYaw) * cosf(_cameraPitch)} * _cameraDistance);
    _camera.setTarget(_centerPoint);
    _camera.refresh(dt);
}