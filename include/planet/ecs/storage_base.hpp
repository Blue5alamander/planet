#pragma once


#include <planet/ecs/entity_id.hpp>


namespace planet::ecs {


    /// Base class for the entity storage
    template<typename Storage>
    class base_entities {
      public:
        virtual ~base_entities() = default;
        virtual entity_id create() = 0;
    };


}
