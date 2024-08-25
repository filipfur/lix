#pragma once

#include <cstdint>
#include <vector>

namespace ecs {
using Entity = uint32_t;

class EntityRegistry {
  public:
    EntityRegistry() : _componentMasks{}, _nextId{0} {}

    virtual ~EntityRegistry() noexcept {}

    EntityRegistry(const EntityRegistry &other) = delete;

    EntityRegistry(EntityRegistry &&other) = delete;

    EntityRegistry &operator=(const EntityRegistry &other) = delete;

    EntityRegistry &operator=(EntityRegistry &&other) = delete;

    void addComponent(uint32_t id, uint32_t componentId) {
        _componentMasks[id] |= componentId;
    }

    void removeComponent(uint32_t id, uint32_t componentId) {
        _componentMasks[id] &= ~componentId;
    }

    bool hasComponents(uint32_t id, uint32_t component) const {
        return (_componentMasks[id] & component) == component;
    }

    static Entity createEntity() { return instance().create(); }

    static EntityRegistry &instance();

    template <typename T> static typename T::value_type *get(uint32_t id) {
        auto &instance = ecs::EntityRegistry::instance();
        if (!instance.hasComponents(id, T::bitSignature())) {
            return nullptr;
        }
        return &T::get(id);
    }

    template <typename T>
    static void set(uint32_t id, typename T::value_type &t) {
        T::set(t, id);
    }

  private:
    Entity create() {
        Entity e = _nextId++;
        _componentMasks.emplace_back(0);
        return e;
    }

    std::vector<uint32_t> _componentMasks;
    Entity _nextId;
};
} // namespace ecs