#pragma once


#include <cstdint>


namespace planet::ecs {


    template<typename... Storages>
    class entities;

    class entity;

    class entity_id;

    template<typename... Components>
    class storage;

    class weak_entity_id;

    using mask_type = std::uint64_t;


    namespace detail {
        class entity;
        struct entity_lookup;
    }


}
