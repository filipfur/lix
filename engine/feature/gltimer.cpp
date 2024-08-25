#include "gltimer.h"

lix::Timer::Timer() : _duration{}, _active{false}, _expired{} {}

lix::Timer::Timer(Time::Raw duration)
    : _duration{duration}, _active{true}, _expired{Time::raw() + duration} {}

bool lix::Timer::active() const { return _active; }

lix::Time::Raw lix::Timer::timeLeft() const { return _expired - Time::raw(); }

float lix::Timer::progress() const {
    float x =
        1.0f - static_cast<float>(timeLeft()) / static_cast<float>(_duration);
    return x < 0.0f ? 0.0f : x;
}

bool lix::Timer::elapsed() const { return _active && Time::raw() >= _expired; }

bool lix::Timer::cancel() {
    if (_active) {
        _active = false;
        return true;
    }
    return false;
}

void lix::Timer::reset() {
    _active = true;
    _expired = Time::millseconds() + _duration;
}

void lix::Timer::set(Time::Raw duration) {
    _duration = duration;
    _active = true;
    _expired = Time::millseconds() + _duration;
}