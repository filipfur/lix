#include "unit_test.h"

#include "gltime.h"
#include "gltimer.h"
#include "primer.h"
#include "response.h"

void TEST() {
    static lix::Timer timer1Hz{lix::Time::fromHz<1>()};
    static lix::Timer timer2Hz{lix::Time::fromHz<2>()};
    static lix::Timer timer50Hz{lix::Time::fromHz<50>()};

    int counter1Hz{};
    int counter2Hz{};
    int counter50Hz{};

    float dt = 0.01f;
    for (float t{0.0f}; t <= 10.0f; t += dt) {
        if (timer1Hz.elapsed()) {
            ++counter1Hz;
            timer1Hz.reset();
        }
        if (timer2Hz.elapsed()) {
            ++counter2Hz;
            timer2Hz.cancel();
        }
        if (timer50Hz.elapsed()) {
            ++counter50Hz;
            timer50Hz.reset();
        }
        lix::Time::increment(10);
    }
    EXPECT_EQ(counter1Hz, 9);
    EXPECT_EQ(counter2Hz, 1); // was canceled
    EXPECT_EQ(counter50Hz, 499);

    for (float t{}; t < 1193.0f * 3600.0f; t += 60.0f) {
        auto t0 = lix::Time::millseconds();
        lix::Time::increment(60 * 1000);
        auto t1 = lix::Time::millseconds();
        EXPECT_LT(t0, t1);
    }

    auto t0 = lix::Time::millseconds();
    lix::Time::increment(3600 * 1000);
    auto t1 = lix::Time::millseconds();
    EXPECT_LT(t1, t0); // uint32_t overflow
}