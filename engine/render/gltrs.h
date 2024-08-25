#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace lix {
class TRS {
  public:
    TRS();

    TRS(const glm::vec3 &translation);
    TRS(const glm::vec3 &translation, const glm::quat &rotation,
        const glm::vec3 &scale);

    virtual ~TRS() noexcept;

    TRS(const TRS &other);
    TRS &operator=(const TRS &other);

    TRS(TRS &&other);
    TRS &operator=(TRS &&other);

    virtual const glm::vec3 &translation() const;
    virtual lix::TRS *setTranslation(const glm::vec3 &translation);
    virtual lix::TRS *applyTranslation(const glm::vec3 &translation);

    virtual const glm::quat &rotation() const;
    virtual lix::TRS *setRotation(const glm::quat &rotation);
    virtual lix::TRS *applyRotation(const glm::quat &translation);

    virtual const glm::vec3 &scale() const;
    virtual lix::TRS *setScale(const glm::vec3 &scale);
    virtual lix::TRS *applyScale(const glm::vec3 &scale);

    const glm::mat4 &rotationMatrix() const;
    const glm::mat4 &modelMatrix();

    bool rotationVersionSync(uint32_t &version) const;
    bool modelVersionSync(uint32_t &version) const;

  protected:
    virtual void invalidate();
    virtual bool updateModelMatrix();

  private:
    glm::vec3 _translation{0.0f};
    glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 _scale{1.0f};

    bool _invalid{false};
    glm::mat4 _rotationMatrix{glm::mat4_cast(_rotation)};
    uint32_t _rotationMatrixVersion{0};
    glm::mat4 _modelMatrix{1.0f};
    uint32_t _modelMatrixVersion{0};
};
} // namespace lix