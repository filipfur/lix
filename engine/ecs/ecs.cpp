#include "ecscomponent.h"

uint32_t ecs::_nextComponentNumber{0};

ecs::EntityRegistry &ecs::EntityRegistry::instance() {
    static ecs::EntityRegistry entityRegistry;
    return entityRegistry;
}