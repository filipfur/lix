#pragma once

#include "gltime.h"

namespace lix
{
    class Timer
    {
    public:
        Timer();
        Timer(Time::Raw duration);

        bool active() const;
        Time::Raw timeLeft() const;
        float progress() const;
        bool elapsed() const;
        bool cancel();
        void reset();
        void set(Time::Raw duration);
        
    private:
        Time::Raw _duration;
        bool _active;
        Time::Raw _expired;
    };
};