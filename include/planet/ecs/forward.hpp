#pragma once


#include <cstdint>


namespace planet::ecs {


    template<typename... Storages>
    class entities;

    struct entity_lookup;

    template<typename... Components>
    class storage;

    using mask_type = std::uint64_t;


    namespace detail {
        class entity;
    }


}
