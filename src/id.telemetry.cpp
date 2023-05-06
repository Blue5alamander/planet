#include <planet/telemetry/id.hpp>

#include <atomic>


/// ## `planet::telemetry::id`


namespace {
    std::atomic<std::uint64_t> next_id = {};
}


planet::telemetry::id::id() : m_name{std::to_string(++next_id)} {}
