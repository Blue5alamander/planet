#include <planet/log.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/string.hpp>
#include <planet/telemetry.hpp>

#include <cmath>
#include <mutex>
#include <vector>


namespace {
    template<typename... Fields>
    bool load_performance_measurement(
            planet::telemetry::performance::measurements &pd,
            std::string const &name,
            std::string_view const box_name,
            Fields &...fields) {
        if (auto d = pd.find(name); d != pd.end()) {
            d->second.check_name_or_throw(box_name);
            d->second.fields(fields...);
            d->second.check_empty_or_throw();
            return true;
        } else {
            return false;
        }
    }
}


/// ## `planet::telemetry::counter`


bool planet::telemetry::counter::save(serialise::save_buffer &ab) {
    auto const c = count.load();
    if (c != 0) {
        ab.save_box(box, name(), c);
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::counter::load(measurements &pd) {
    std::int64_t c;
    if (load_performance_measurement(pd, name(), box, c)) {
        count += c;
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


/// ## `planet::telemetry::detail`


namespace {
    constexpr std::string_view measurements_box{"_p:t:mb"};
}


std::size_t planet::telemetry::detail::save_performance(
        serialise::save_buffer &sb, std::span<performance *> pcs) {
    std::size_t count{};
    sb.save_box_lambda(measurements_box, [&]() {
        for (auto p : pcs) {
            if (p->save(sb)) { ++count; }
        }
    });
    return count;
}


std::size_t planet::telemetry::detail::load_performance(
        serialise::load_buffer &lb, std::span<performance *> const pcs) {
    auto box = serialise::load_type<serialise::box>(lb);
    box.check_name_or_throw(measurements_box);
    auto map = performance::saved_measurements(box.content);
    std::size_t count{};
    for (auto p : pcs) {
        if (p->load(map)) { ++count; }
    }
    return count;
}


/// ## `planet::telemetry::exponential_decay`


planet::telemetry::exponential_decay::exponential_decay(
        std::string_view const n, std::size_t const half_life)
: performance{n},
  decay_rate{std::pow(2.0, -1.0 / static_cast<double>(half_life))} {}


void planet::telemetry::exponential_decay::add_measurement(double const m) {
    auto const a = (1.0 - decay_rate) * m;
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * decay_rate + a)) {}
}


bool planet::telemetry::exponential_decay::save(serialise::save_buffer &ab) {
    auto const c = m_value.load();
    if (c) {
        ab.save_box(box, name(), c);
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::exponential_decay::load(measurements &pd) {
    double c;
    if (load_performance_measurement(pd, name(), box, c)) {
        m_value.store(c);
        return true;
    } else {
        return false;
    }
}


namespace {
    auto const exponential_decay_print = planet::log::format(
            planet::telemetry::exponential_decay::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                double value;
                box.named(
                        planet::telemetry::exponential_decay::box, name, value);
                os << name << " = " << value;
            });
}


/// ## `planet::telemetry::id`


namespace {
    std::atomic<std::uint64_t> next_id = {};
    std::string create_suffix() { return std::to_string(++next_id); }
}


planet::telemetry::id::id() : m_name{create_suffix()} {}


planet::telemetry::id::id(std::string n, suffix const s)
: m_name{s == suffix::yes ? (n + "__" + create_suffix()) : std::move(n)} {}


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


planet::telemetry::performance::performance(std::string_view const n)
: id{n, id::suffix::no} {
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


auto planet::telemetry::performance::saved_measurements(
        serialise::load_buffer &lb) -> measurements {
    measurements map;
    while (not lb.empty()) {
        auto box = serialise::load_type<serialise::box>(lb);
        std::string name;
        serialise::load(box.content, name);
        map.insert(std::pair{std::move(name), std::move(box)});
    }
    return map;
}


/// ## `planet::telemetry::real_time_decay`


namespace {
    double decay_factor(std::chrono::nanoseconds const ns, double half_life) {
        auto const ts = static_cast<double>(ns.count());
        return std::pow(2.0, -ts / half_life);
    }
}


void planet::telemetry::real_time_decay::add_measurement(double const m) {
    auto const decay = decay_factor(last.checkpoint(), half_life);
    auto const a = m * (1 - decay);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * decay + a)) {}
}


bool planet::telemetry::real_time_decay::save(serialise::save_buffer &ab) {
    auto const decay = decay_factor(last.checkpoint(), half_life);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * decay)) {}
    if (ov != 0.0) {
        ab.save_box(box, name(), ov);
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::real_time_decay::load(measurements &pd) {
    double c;
    if (load_performance_measurement(pd, name(), box, c)) {
        add_measurement(c);
        return true;
    } else {
        return false;
    }
}


namespace {
    auto const real_time_decay_print = planet::log::format(
            planet::telemetry::real_time_decay::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                double value;
                box.named(planet::telemetry::real_time_decay::box, name, value);
                os << name << " = " << value;
            });
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
    if (v != 0.0) {
        ab.save_box(box, name(), m_value.load());
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::real_time_rate::load(measurements &pd) {
    double c;
    if (load_performance_measurement(pd, name(), box, c)) {
        last.checkpoint();
        m_value.store(c);
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
                double value;
                box.named(planet::telemetry::real_time_rate::box, name, value);
                os << name << " = " << value << "Hz";
            });
}
