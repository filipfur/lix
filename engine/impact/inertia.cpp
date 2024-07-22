#include "inertia.h"

glm::mat3 lix::cubeInertiaTensor(float mass, float sideLength) {
    float inertia = mass * sideLength * sideLength;
    return {
        0.66f * inertia, -0.25f * inertia, -0.25f * inertia,
        -0.25f * inertia, 0.66f * inertia, -0.25f * inertia,
        -0.25f * inertia, -0.25f * inertia, 0.66f * inertia
    };
}