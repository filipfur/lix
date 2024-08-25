#pragma once

#include "glcamera.h"
#include "glinputadapter.h"
#include "glm/glm.hpp"
#include "gltypes.h"

namespace lix {
class Editor : public lix::InputAdapter {
  public:
    Editor(const glm::mat4 &perspective, const glm::vec2 &resolution,
           float cameraDistance = 4.0f);

    virtual bool onMouseDown(lix::KeySym key, lix::KeyMod mod) override;
    virtual bool onMouseUp(lix::KeySym key, lix::KeyMod mod) override;
    virtual bool onMouseMove(float x, float y, float xrel, float yrel) override;
    virtual bool onMouseWheel(float x, float y) override;
    virtual bool onKeyDown(lix::KeySym key, lix::KeyMod mod) override;
    virtual bool onKeyUp(lix::KeySym key, lix::KeyMod mod) override;

    void setSubjectNode(lix::Node *subjectNode_) {
        _subjectNode = subjectNode_;
    }

    Camera &camera() { return _camera; }

    enum class Transformation { TRANSLATE, ROTATE };
    virtual void onSubjectTransformed(lix::Node *subject,
                                      Transformation transformation) = 0;

    void refresh(float dt);

  private:
    enum TranslationMode {
        TRANS_NONE,
        TRANS_X,
        TRANS_Y,
        TRANS_Z
    } _translationMode{TRANS_NONE};
    enum EditMode { EDIT_NONE, EDIT_TRANS, EDIT_ROT } _editMode{EDIT_NONE};

    void setEditMode(EditMode mode) {
        _editMode = _editMode == mode ? EDIT_NONE : mode;
        _translationMode = TRANS_NONE;
    }

    void setTranslationMode(TranslationMode mode) {
        if (_editMode == EDIT_NONE || _subjectNode == nullptr) {
            return;
        }
        _subjectNode->setTranslation(_prevPos);
        _translationMode = _translationMode == mode ? TRANS_NONE : mode;
    }

    void setCameraDistance(float distance) { _cameraDistance = distance; }

    float cameraDistance() const { return _cameraDistance; }

    Camera _camera;
    glm::vec2 _resolution;
    lix::Node *_subjectNode;
    glm::vec3 _prevPos;
    bool _lmb{false};
    float _cameraYaw{0.0f};
    float _cameraPitch{0.0f};
    float _cameraDistance{4.0f};
    glm::vec3 _centerPoint{0.0f, 0.0f, 0.0f};
};
} // namespace lix