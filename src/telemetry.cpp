#include <planet/serialise/base_types.hpp>
#include <planet/serialise/string.hpp>
#include <planet/telemetry.hpp>

#include <cmath>
#include <mutex>
#include <vector>


/// ## `planet::telemetry::counter`


bool planet::telemetry::counter::save(serialise::save_buffer &ab) {
    ab.save_box("_p:t:counter", name(), count.load());
    return true;
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
        static auto p = []() {
            std::vector<planet::telemetry::performance *> v;
            v.reserve(128);
            return v;
        }();
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


std::size_t planet::telemetry::performance::current_values(
        serialise::save_buffer &ab) {
    std::scoped_lock _{g_mtx};
    auto &p = g_perfs();
    return std::count_if(
            p.begin(), p.end(), [&](auto c) { return c->save(ab); });
}


/// ## `planet::telemetry::real_time_rate`


void planet::telemetry::real_time_rate::reading(float const m) {
    std::chrono::nanoseconds ns{last.checkpoint()};
    auto const rt = std::pow(2.0, -static_cast<double>(ns.count()) / half_life);
    auto const a = m * (1.0 - rt);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * rt + a)) {}
}


bool planet::telemetry::real_time_rate::save(serialise::save_buffer &ab) {
    reading({});
    ab.save_box("_p:t:rate", name(), m_value.load());
    return true;
}
