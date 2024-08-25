#pragma once

#include <fstream>
#include <iostream>

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#define print_var(var) std::cout << #var << "=" << var << std::endl;

template <typename T>
static void printError(const char *labelA, const char *labelB, const char *op,
                       const T &valueA, const T &valueB) {
    std::cerr << "ERROR: " << labelA << ' ' << op << ' ' << labelB
              << " expands to " << valueA << ' ' << op << ' ' << valueB
              << std::endl;
    exit(1);
}

#define EXPECT_EQ(a, b)                                                        \
    if (a != b)                                                                \
        printError(#a, #b, "==", a, b);
#define EXPECT_LT(a, b)                                                        \
    if (a >= b)                                                                \
        printError(#a, #b, "<", a, b);

inline std::ostream &operator<<(std::ostream &os, const glm::vec3 &v) {
    os << glm::to_string(v);
    return os;
}

void TEST();