#pragma once


#include <planet/ecs/forward.hpp>

#include <felspar/exceptions.hpp>


namespace planet::ecs::detail {


    [[noreturn]] void throw_no_entities_instance(
            felspar::source_location const & =
                    felspar::source_location::current());
    [[noreturn]] void throw_component_type_not_valid(
            entity_id const &,
            std::type_info const &,
            felspar::source_location const & =
                    felspar::source_location::current());
    [[noreturn]] void throw_component_not_present(
            entity_id const &,
            std::type_info const &,
            felspar::source_location const & =
                    felspar::source_location::current());
    [[noreturn]] void throw_entity_not_valid(
            entity_id const &,
            felspar::source_location const & =
                    felspar::source_location::current());
    [[noreturn]] void throw_wrong_generation(
            std::size_t eid,
            std::size_t expected_generation,
            std::size_t actual_generation,
            felspar::source_location const & =
                    felspar::source_location::current());


}
