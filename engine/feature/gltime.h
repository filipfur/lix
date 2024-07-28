#pragma once

#include <cstdint>

namespace lix
{
    namespace Time
    {
        using Raw = uint32_t;
        
        Raw raw();
        Raw millseconds();
        float seconds();

        template <Raw hz>
        inline static constexpr Raw fromHz()
        {
            static_assert(1000 % hz == 0);
            return 1000 / hz;
        }

        inline static constexpr Raw fromSeconds(float s)
        {
            return static_cast<Raw>(s * 1e3f);
        }

        inline static constexpr Raw fromMilliseconds(Raw ms)
        {
            return ms;
        }

        inline static constexpr float toSeconds(Raw raw)
        {
            return raw * 1e-3f;
        }

        void increment(Raw delta_ms);
    }
}