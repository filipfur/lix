#pragma once

#include <iostream>
#include <vector>

namespace lix {
class Animation {
  public:
    struct Frame {
        int id;
        float duration;
    };

    Animation(const std::string &name, const std::vector<Frame> &frames)
        : _name{name}, _frames{frames} {
        _timeToNext = _frames.front().duration;
    }

    void update(float dt) {
        while (dt > 0) {
            float ddt = std::min(dt, _timeToNext);
            _timeToNext -= ddt;
            if (_timeToNext <= 0) {
                _frameIndex = nextFrameIndex();
                _timeToNext = _frames[_frameIndex].duration;
            }
            dt -= ddt;
        }
    }

    int currentFrame() const { return _frames.at(_frameIndex).id; }

    int nextFrame() const { return _frames.at(nextFrameIndex()).id; }

    int previousFrame() const { return _frames.at(previousFrameIndex()).id; }

  private:
    int nextFrameIndex() const { return (_frameIndex + 1) % _frames.size(); }

    int previousFrameIndex() const {
        return (_frameIndex - 1) % _frames.size();
    }

    const std::string _name;
    const std::vector<Frame> _frames;
    int _frameIndex{0};
    float _timeToNext;
};
} // namespace lix