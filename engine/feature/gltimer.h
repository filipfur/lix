#pragma once

#include "gltime.h"

namespace lix
{
    class Timer
    {
    public:
        Timer();
        Timer(Time::Raw duration_ms);

        bool elapsed();
        bool cancel();
        void reset();

    private:
        const Time::Raw _duration;
        bool _active;
        Time::Raw _expired;
    };
};