#pragma once

#include "glskinanimation.h"
#include <map>
#include <memory>
#include <vector>

namespace lix {
class Skin {
  public:
    std::vector<glm::mat4> &inverseBindMatrices();

    std::vector<lix::Node *> &joints();

    lix::Node *joint(size_t index);

    void addAnimation(const std::string &key,
                      std::shared_ptr<lix::SkinAnimation> animation);

    std::map<std::string, std::shared_ptr<lix::SkinAnimation>> &animations();

    std::map<std::string, std::shared_ptr<lix::SkinAnimation>>::iterator
    currentAnimation();
    void setCurrentAnimation(
        std::map<std::string, std::shared_ptr<lix::SkinAnimation>>::iterator
            currentAnimation);

  private:
    std::vector<glm::mat4> _inverseBindMatrices;
    std::vector<lix::Node *> _joints;
    std::map<std::string, std::shared_ptr<lix::SkinAnimation>> _animations;
    std::map<std::string, std::shared_ptr<lix::SkinAnimation>>::iterator
        _currentAnimation;
};

using SkinAnimationIterator =
    std::map<std::string, std::shared_ptr<lix::SkinAnimation>>::iterator;
} // namespace lix