#include "gltrs.h"

#include <utility>

lix::TRS::TRS()
{
}

lix::TRS::TRS(const glm::vec3 &translation)
    : TRS{translation, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}}
{
}

lix::TRS::TRS(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale)
    : _translation{translation}, _rotation{rotation}, _scale{scale}, _invalid{true}
{
    updateModelMatrix();
}

lix::TRS::~TRS() noexcept
{
}

lix::TRS::TRS(const TRS &other)
    : _translation{other._translation}, _rotation{other._rotation}, _scale{other._scale},
      _invalid{other._invalid}, _rotationMatrix{other._rotationMatrix}, _modelMatrix{other._modelMatrix}
{
}

lix::TRS &lix::TRS::operator=(const TRS &other)
{
    _translation = other._translation;
    _rotation = other._rotation;
    _scale = other._scale;
    _invalid = other._invalid;
    _modelMatrix = other._modelMatrix;
    _rotationMatrix = other._rotationMatrix;
    return *this;
}

lix::TRS::TRS(TRS &&other)
    : _translation{std::move(other._translation)}, _rotation{std::move(other._rotation)}, _scale{std::move(other._scale)},
      _invalid{other._invalid}, _rotationMatrix{std::move(other._rotationMatrix)}, _modelMatrix{std::move(other._modelMatrix)}
{
}

lix::TRS &lix::TRS::operator=(TRS &&other)
{
    _translation = std::move(other._translation);
    _rotation = std::move(other._rotation);
    _scale = std::move(other._scale);
    _invalid = other._invalid;
    _modelMatrix = std::move(other._modelMatrix);
    _rotationMatrix = std::move(other._rotationMatrix);
    return *this;
}

const glm::vec3 &lix::TRS::translation() const { return _translation; }
lix::TRS *lix::TRS::setTranslation(const glm::vec3 &translation)
{
    invalidate();
    _translation = translation;
    return this;
}
lix::TRS *lix::TRS::applyTranslation(const glm::vec3 &translation)
{
    invalidate();
    _translation += translation;
    return this;
}

const glm::quat &lix::TRS::rotation() const { return _rotation; }
lix::TRS *lix::TRS::setRotation(const glm::quat &rotation)
{
    invalidate();
    _rotation = rotation;
    _rotationMatrix = glm::mat4_cast(_rotation);
    ++_rotationMatrixVersion;
    return this;
}
lix::TRS *lix::TRS::applyRotation(const glm::quat &rotation)
{
    invalidate();
    _rotation *= rotation;
    _rotationMatrix = glm::mat4_cast(_rotation);
    ++_rotationMatrixVersion;
    return this;
}

const glm::vec3 &lix::TRS::scale() const { return _scale; }
lix::TRS *lix::TRS::setScale(const glm::vec3 &scale)
{
    invalidate();
    _scale = scale;
    return this;
}
lix::TRS *lix::TRS::applyScale(const glm::vec3 &scale)
{
    invalidate();
    _scale += scale;
    return this;
}

const glm::mat4& lix::TRS::rotationMatrix() const
{
    return _rotationMatrix;
}

const glm::mat4 &lix::TRS::modelMatrix()
{
    updateModelMatrix();
    return _modelMatrix;
}

bool lix::TRS::rotationVersionSync(uint32_t& version) const
{
    if(version == _rotationMatrixVersion)
    {
        return true;
    }
    version = _rotationMatrixVersion;
    return false;
}

bool lix::TRS::modelVersionSync(uint32_t& version) const
{
    if(version == _modelMatrixVersion)
    {
        return true;
    }
    version = _modelMatrixVersion;
    return false;
}

bool lix::TRS::updateModelMatrix()
{
    if (_invalid)
    {
        _modelMatrix = glm::scale(glm::translate(glm::mat4{1.0f}, _translation) * _rotationMatrix, _scale);
        _invalid = false;
        ++_modelMatrixVersion;
        return true;
    }
    return false;
}

void lix::TRS::invalidate()
{
    _invalid = true;
}
