#include "glskin.h"

std::vector<glm::mat4>& lix::Skin::inverseBindMatrices()
{
    return _inverseBindMatrices;
}

std::vector<lix::Node*>& lix::Skin::joints()
{
    return _joints;
}

lix::Node* lix::Skin::joint(size_t index)
{
    return _joints.at(index);
}

void lix::Skin::addAnimation(const std::string& key,
    std::shared_ptr<lix::SkinAnimation> animation)
{
    _animations.emplace(key, animation);
}