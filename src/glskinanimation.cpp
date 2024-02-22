#include "glskinanimation.h"

#include "glnode.h"

void lix::SkinAnimation::setTime(float time)
{
    _time = time;
}
float lix::SkinAnimation::time() const
{
    return _time;
}

void lix::SkinAnimation::setStart(float start)
{
    _start = start;
}
float lix::SkinAnimation::start() const
{
    return _start;
}

void lix::SkinAnimation::setEnd(float end)
{
    _end = end;
}
float lix::SkinAnimation::end() const
{
    return _end;
}

std::list<lix::SkinAnimation::Channel>& lix::SkinAnimation::channels()
{
    return _channels;
}

void lix::SkinAnimation::update(float dt)
{
    _time += dt;
    float delta = _end - _time;
    if(delta < 0)
    {
        _time = _start - delta;
    }
    for(const auto& channel : _channels)
    {
        switch(channel.type())
        {
            case Channel::TRANSLATION:
                channel.node()->setTranslation(channel.translation(_time));
                break;
            case Channel::ROTATION:
                channel.node()->setRotation(channel.rotation(_time));
                break;
            case Channel::SCALE:
                channel.node()->setScale(channel.scale(_time));
                break;
            default:
                throw std::runtime_error("unknown channel type");
        }
    }
}

inline glm::vec3 interpolateVec3(const std::map<float, glm::vec3>& vec, float t)
{
    assert(t >= 0);
   
    auto cur = vec.upper_bound(t);
    if(cur == vec.end())
    {
        --cur;
        return cur->second;
    }
    else
    {
        auto next = cur;
        assert(cur != vec.begin());
        --cur;
        float a = t - cur->first;
        float b = next->first - t;
        float c = a + b;
        return glm::mix(cur->second, next->second, a / c);
    }
}

glm::vec3 lix::SkinAnimation::Channel::translation(float time) const
{
    return interpolateVec3(_translations, time);
}

glm::quat lix::SkinAnimation::Channel::rotation(float t) const
{
    assert(t >= 0);
   
    auto cur = _rotations.upper_bound(t);
    if(cur == _rotations.end())
    {
        --cur;
        return cur->second;
    }
    else
    {
        auto next = cur;
        --cur;
        float a = t - cur->first;
        float b = next->first - t;
        float c = a + b;
        return glm::slerp(cur->second, next->second, a / c);
    }
}

glm::vec3 lix::SkinAnimation::Channel::scale(float time) const
{
    return interpolateVec3(_scales, time);
}

std::map<float, glm::vec3>& lix::SkinAnimation::Channel::translations()
{
    return _translations;
}

std::map<float, glm::quat>& lix::SkinAnimation::Channel::rotations()
{
    return _rotations;
}

std::map<float, glm::vec3>& lix::SkinAnimation::Channel::scales()
{
    return _scales;
}

void lix::SkinAnimation::Channel::setType(lix::SkinAnimation::Channel::Type type)
{
    _type = type;
}

lix::SkinAnimation::Channel::Type lix::SkinAnimation::Channel::type() const
{
    return _type;
}

void lix::SkinAnimation::Channel::setNode(lix::Node* node)
{
    _node = node;
}

lix::Node* lix::SkinAnimation::Channel::node() const
{
    return _node;
}