#pragma once

#include <functional>

namespace ecs {
template <class... T> class Slice {
  public:
    static void
    forEach(std::vector<ecs::Entity> &entities,
            std::function<void(ecs::Entity, typename T::value_type &...)>
                callback) {
        auto mask = Slice<T...>::mask();
        for (auto &entity : entities) {
            if (EntityRegistry::instance().hasComponents(entity, mask)) {
                (callback(entity, T::get(entity)...));
            }
        }
    }

    static uint32_t mask() { return (T::bitSignature() + ...); }
};
} // namespace ecs