#pragma once

#include <typeinfo>
#include <iostream>
#include <vector>

#include "ecscomponent.h"
#include "ecsentity.h"
#include "ecsconstants.h"
#include "ecsslice.h"

namespace ecs
{

    template <class... T>
    class System : public Slice<T...>
    { 
        public:
            System() : _versions{}
            {

            }

            void update(std::vector<ecs::Entity>& entities,
                std::function<void(ecs::Entity, typename T::value_type&...)> callback)
            {
                auto mask = Slice<T...>::mask();
                for(ecs::Entity entity : entities)
                {
                    if(EntityRegistry::instance().hasComponents(entity, mask))
                    {
                        //bool test = ((_versions[entityId][T::_number] != T::version(entityId)) && ...);
                        bool test = (T::compare(entity, _versions[entity][T::_number]) && ...);
#ifdef ECS_TRACE
                        //std::cout << "ecs::System: Testing mask=" << mask << ", entity=" << entityId << ": " << (test ? "[ ]" : "[X]") << std::endl;
#endif
                        if(!test) // Check if NOT all versions match
                        {
#ifdef ECS_TRACE
                            std::cout << "ecs::System: Updating mask=" << mask << ", entity=" << entityId << std::endl;
#endif
                            (callback(entity,
                                T::get(entity)...
                            ));
                            (T::increment(entity, std::is_const_v<T> == false),...); // Increment version of NON-const components
                            (setVersion(entity, T::_number, T::version(entity)), ...); // Update own version to match latest-greatest.
                        }
                    }
                }
            }

            void setVersion(Entity entity, size_t componentNumber, uint8_t version)
            {
                _versions[entity][componentNumber] = version;
            }

            virtual ~System() noexcept
            {

            }

        private:
            uint8_t _versions[ecs::GLOBAL_MAX_ENTITIES][ecs::GLOBAL_MAX_COMPONENTS];
    };
}