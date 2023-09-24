#include <planet/ecs/storage.hpp>

#include <felspar/exceptions.hpp>


/// ## `planet::ecs::detail`


void planet::ecs::detail::throw_no_entities_instance(
        felspar::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "The entities storage must be part of an entities structure before "
            "it can be used",
            loc};
}


void planet::ecs::detail::throw_component_type_not_valid(
        felspar::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "The provided type doesn't match a component type", loc};
}


void planet::ecs::detail::throw_component_not_present(
        felspar::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "This entity doesn't have that component at this time", loc};
}


void planet::ecs::detail::throw_entity_not_valid(
        felspar::source_location const &loc) {
    throw felspar::stdexcept::logic_error{"This entity is not valid", loc};
}


namespace {
    planet::telemetry::counter create_count{"planet_ecs_entities_created"};
    planet::telemetry::counter destroy_count{"planet_ecs_entities_destroyed"};
}
void planet::ecs::detail::count_create_entity() { ++create_count; }
void planet::ecs::detail::count_destroy_entity() { ++destroy_count; }
