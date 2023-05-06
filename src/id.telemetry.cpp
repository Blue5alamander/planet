#include <planet/telemetry/id.hpp>

#include <atomic>


/// ## `planet::telemetry::id`


namespace {
    std::atomic<std::uint64_t> next_id = {};

    std::string suffix() { return std::to_string(++next_id); }
}


planet::telemetry::id::id() : m_name{suffix()} {}


planet::telemetry::id::id(std::string_view const n)
: m_name{std::string{n} + "__" + suffix()} {}
