#include <planet/log.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/string.hpp>
#include <planet/telemetry.hpp>

#include <cmath>
#include <mutex>
#include <vector>


/// ## `planet::telemetry::counter`


bool planet::telemetry::counter::save(serialise::save_buffer &ab) {
    auto const c = count.load();
    if (c != 0) {
        ab.save_box(box, name(), count.load());
        return true;
    } else {
        return false;
    }
}
namespace {
    auto const counter_print = planet::log::format(
            planet::telemetry::counter::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                std::int64_t count;
                box.named(planet::telemetry::counter::box, name, count);
                os << name << " = " << count;
            });
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


void planet::telemetry::real_time_rate::tick() {
    std::chrono::nanoseconds ns{last.checkpoint()};
    auto const ts = static_cast<double>(ns.count());
    auto const m = 1e9 / ts;
    auto const rt = std::pow(2.0, -ts / half_life);
    auto const a = m * (1.0 - rt);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * rt + a)) {}
}


bool planet::telemetry::real_time_rate::save(serialise::save_buffer &ab) {
    auto const v = m_value.load();
    if (v != 0.0f) {
        ab.save_box(box, name(), m_value.load());
        return true;
    } else {
        return false;
    }
}
namespace {
    auto const real_time_rate_print = planet::log::format(
            planet::telemetry::real_time_rate::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                float value;
                box.named(planet::telemetry::real_time_rate::box, name, value);
                os << name << " = " << value << "Hz";
            });
}
