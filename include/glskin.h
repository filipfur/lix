#pragma once

#include "glskinanimation.h"

namespace lix
{
    class Skin
    {
    public:
        std::vector<glm::mat4>& inverseBindMatrices();

        std::vector<lix::Node*>& joints();

        lix::Node* joint(size_t index);

        void addAnimation(const std::string& key,
            std::shared_ptr<lix::SkinAnimation> animation);

    private:
        std::vector<glm::mat4> _inverseBindMatrices;
        std::vector<lix::Node*> _joints;
        std::map<std::string, std::shared_ptr<lix::SkinAnimation>> _animations;
    };
}