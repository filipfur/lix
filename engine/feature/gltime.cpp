#include "gltime.h"

static lix::Time::Raw _time{};

float lix::Time::seconds() { return _time * 1e-3f; }

lix::Time::Raw lix::Time::raw() { return _time; }

lix::Time::Raw lix::Time::millseconds() { return _time; }

void lix::Time::increment(lix::Time::Raw delta_ms) { _time += delta_ms; }