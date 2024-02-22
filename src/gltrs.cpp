#include "gltrs.h"

lix::TRS::TRS()
{
    
}

lix::TRS::TRS(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale)
    : _translation{translation}, _rotation{rotation}, _scale{scale}, _invalid{true}
{
    updateModelMatrix();
}

lix::TRS::~TRS() noexcept
{
    
}

lix::TRS::TRS(const TRS& other)
    : _translation{other._translation}, _rotation{other._rotation}, _scale{other._scale},
    _invalid{other._invalid}, _model{other._model}, _rotMat{other._rotMat}
{

}

lix::TRS& lix::TRS::operator=(const TRS& other)
{
    _translation = other._translation;
    _rotation = other._rotation;
    _scale = other._scale;
    _invalid = other._invalid;
    _model = other._model;
    _rotMat = other._rotMat;
    return *this;
}

const glm::vec3& lix::TRS::translation() const { return _translation; }
lix::TRS* lix::TRS::setTranslation(const glm::vec3& translation) { invalidate(); _translation = translation; return this; }
lix::TRS* lix::TRS::applyTranslation(const glm::vec3& translation) { invalidate(); _translation += translation; return this; }

//using position = translation;
//using setPosition = setTranslation;

const glm::quat& lix::TRS::rotation() const { return _rotation; }
lix::TRS* lix::TRS::setRotation(const glm::quat& rotation) { invalidate(); _rotation = rotation; _rotMat = glm::mat4_cast(_rotation); return this; }
lix::TRS* lix::TRS::applyRotation(const glm::quat& rotation) { invalidate(); _rotation *= rotation; _rotMat = glm::mat4_cast(_rotation); return this; }

const glm::vec3& lix::TRS::scale() const { return _scale; }
lix::TRS* lix::TRS::setScale(const glm::vec3& scale) { invalidate(); _scale = scale; return this; }
lix::TRS* lix::TRS::applyScale(const glm::vec3& scale) { invalidate(); _scale += scale; return this; }

const glm::mat4& lix::TRS::model()
{ 
    updateModelMatrix();
    return _model;
}

bool lix::TRS::updateModelMatrix()
{
    if(_invalid)
    {
        _model = glm::scale(glm::translate(glm::mat4{1.0f}, _translation) * _rotMat, _scale);
        _invalid = false;
        return true;
    }
    return false;
}

void lix::TRS::invalidate()
{
    _invalid = true;
}
