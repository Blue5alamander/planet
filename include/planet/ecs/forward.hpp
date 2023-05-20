#pragma once


#include <cstdint>


namespace planet::ecs {


    template<typename... Storages>
    class entities;

    class entity;

    struct entity_lookup;


    using mask_type = std::uint64_t;


}
