#include <planet/serialise/base_types.hpp>
#include <planet/telemetry.hpp>

#include <cmath>
#include <mutex>
#include <vector>


/// ## `planet::telemetry::counter`


void planet::telemetry::counter::save(serialise::save_buffer &ab) {
    serialise::save(ab, count.load());
}


/// ## `planet::telemetry::id`


namespace {
    std::atomic<std::uint64_t> next_id = {};

    std::string suffix() { return std::to_string(++next_id); }
}


planet::telemetry::id::id() : m_name{suffix()} {}


planet::telemetry::id::id(std::string_view const n)
: m_name{std::string{n} + "__" + suffix()} {}


/// ## `planet::telemetry::performance`


namespace {
    std::mutex g_mtx;
    auto &g_perfs() {
        static std::vector<planet::telemetry::performance *> p(128);
        return p;
    }
}


planet::telemetry::performance::performance(std::string_view const n) : id{n} {
    std::scoped_lock _{g_mtx};
    g_perfs().push_back(this);
}


planet::telemetry::performance::~performance() {
    std::scoped_lock _{g_mtx};
    auto &p = g_perfs();
    p.erase(std::find(p.begin(), p.end(), this));
}


/// ## `planet::telemetry::rate`


void planet::telemetry::rate::reading(
        float const m, std::chrono::nanoseconds const ns) {
    auto const rt = std::pow(2.0, -static_cast<double>(ns.count()) / half_life);
    auto const a = m * (1.0 - rt);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * rt + a)) {}
}


void planet::telemetry::rate::save(serialise::save_buffer &ab) {
    serialise::save(ab, m_value.load());
}
