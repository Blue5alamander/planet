#include <planet/ecs/storage.hpp>

#include <felspar/exceptions.hpp>


/// ## `planet::ecs::detail`


namespace {
    struct eid_and_type_info : public felspar::stdexcept::logic_error {
        eid_and_type_info(
                std::string_view const m,
                planet::ecs::entity_id const eid,
                std::type_info const &ti,
                felspar::source_location const &loc)
        : felspar::stdexcept::logic_error{
                  std::string{m} + "\nEntity id " + std::to_string(eid.id())
                          + " type index: " + std::string{ti.name()},
                  loc} {}
    };
}


void planet::ecs::detail::throw_no_entities_instance(
        felspar::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "The entities storage must be part of an entities structure before "
            "it can be used",
            loc};
}


void planet::ecs::detail::throw_component_type_not_valid(
        entity_id const &eid,
        std::type_info const &ti,
        felspar::source_location const &loc) {
    throw eid_and_type_info{
            "The provided type doesn't match a component type", eid, ti, loc};
}


void planet::ecs::detail::throw_component_not_present(
        entity_id const &eid,
        std::type_info const &ti,
        felspar::source_location const &loc) {
    throw eid_and_type_info{
            "This entity doesn't have that component at this time", eid, ti,
            loc};
}


void planet::ecs::detail::throw_entity_not_valid(
        entity_id const &eid, felspar::source_location const &loc) {
    throw felspar::stdexcept::logic_error{
            "This entity is not valid\nEntity ID: " + std::to_string(eid.id()),
            loc};
}


namespace {
    planet::telemetry::counter create_count{"planet_ecs_entities_created"};
    planet::telemetry::counter recycle_count{"planet_ecs_entities_recycled"};
    planet::telemetry::counter destroy_count{"planet_ecs_entities_destroyed"};
}
void planet::ecs::detail::count_create_entity() noexcept { ++create_count; }
void planet::ecs::detail::count_recycled_entity() noexcept { ++recycle_count; }
void planet::ecs::detail::count_destroy_entity() noexcept { ++destroy_count; }
