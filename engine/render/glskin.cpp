#include "glskin.h"

std::vector<glm::mat4> &lix::Skin::inverseBindMatrices() {
    return _inverseBindMatrices;
}

std::vector<lix::Node *> &lix::Skin::joints() { return _joints; }

lix::Node *lix::Skin::joint(size_t index) { return _joints.at(index); }

void lix::Skin::addAnimation(const std::string &key,
                             std::shared_ptr<lix::SkinAnimation> animation) {
    _currentAnimation = _animations.emplace(key, animation).first;
}

std::map<std::string, std::shared_ptr<lix::SkinAnimation>> &
lix::Skin::animations() {
    return _animations;
}

std::map<std::string, std::shared_ptr<lix::SkinAnimation>>::iterator
lix::Skin::currentAnimation() {
    return _currentAnimation;
}

void lix::Skin::setCurrentAnimation(
    std::map<std::string, std::shared_ptr<lix::SkinAnimation>>::iterator
        currentAnimation) {
    _currentAnimation = currentAnimation;
}