#include "gltimer.h"

lix::Timer::Timer() : _duration{}, _active{false}, _expired{}
{}

lix::Timer::Timer(Time::Raw duration_ms) : _duration{duration_ms}, _active{true}, _expired{Time::millseconds() + duration_ms}
{}

bool lix::Timer::elapsed()
{
    return _active && Time::millseconds() >= _expired;
}

bool lix::Timer::cancel()
{
    if(_active)
    {
        _active = false;
        return true;
    }
    return false;
}

void lix::Timer::reset()
{
    _active = true;
    _expired = Time::millseconds() + _duration;
}