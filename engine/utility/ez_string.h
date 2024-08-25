#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static inline bool starts_with(const char *str, const char *prefix) {
    size_t lenPrefix = strlen(prefix);
    size_t lenStr = strlen(str);
    if (lenStr < lenPrefix) {
        return false;
    }
    return strncmp(str, prefix, lenPrefix) == 0;
}