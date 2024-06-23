#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace lix
{
    class TRS
    {
    public:
        TRS();

        TRS(const glm::vec3& translation);
        TRS(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale);

        virtual ~TRS() noexcept;
        
        TRS(const TRS& other);
        TRS& operator=(const TRS& other);
        
        TRS(TRS&& other);
        TRS& operator=(TRS&& other);

        virtual const glm::vec3& translation() const;
        virtual lix::TRS* setTranslation(const glm::vec3& translation);
        virtual lix::TRS* applyTranslation(const glm::vec3& translation);

        virtual const glm::quat& rotation() const;
        virtual lix::TRS* setRotation(const glm::quat& rotation);
        virtual lix::TRS* applyRotation(const glm::quat& translation);

        virtual const glm::vec3& scale() const;
        virtual lix::TRS* setScale(const glm::vec3& scale);
        virtual lix::TRS* applyScale(const glm::vec3& scale);

        const glm::mat4& model();

    protected:
        virtual bool updateModelMatrix();

        virtual void invalidate();

        bool invalid()
        {
            return _invalid;
        }

    private:
        glm::vec3 _translation{0.0f};
        glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 _scale{1.0f};

        bool _invalid{false};
        glm::mat4 _model{1.0f};
        glm::mat4 _rotMat{glm::mat4_cast(_rotation)};
    };
}