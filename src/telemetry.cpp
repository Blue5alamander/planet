#include <planet/log.hpp>
#include <planet/serialise/base_types.hpp>
#include <planet/serialise/string.hpp>
#include <planet/telemetry.hpp>

#include <cmath>
#include <vector>


using namespace std::literals;


/// ## `planet::telemetry::counter`


bool planet::telemetry::counter::save(serialise::save_buffer &ab) const {
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
        serialise::save_buffer &sb, std::span<performance const *> pcs) {
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
    auto box = serialise::expect_box(lb);
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
        std::string_view const n,
        std::size_t const half_life,
        std::source_location const &loc)
: performance{n, loc},
  decay_rate{std::pow(2.0, -1.0 / static_cast<double>(half_life))} {}


void planet::telemetry::exponential_decay::add_measurement(
        double const m) noexcept {
    auto const a = (1.0 - decay_rate) * m;
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * decay_rate + a)) {}
}


bool planet::telemetry::exponential_decay::save(
        serialise::save_buffer &ab) const {
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
: m_name{s == suffix::add ? (n + "__" + create_suffix()) : std::move(n)} {}


/// ## `planet::telemetry::max`


void planet::telemetry::max::value(value_type const v) noexcept {
    while (true) {
        auto const old = m_value.load();
        auto const m = std::max(old, v);
        if (m == old) {
            return;
        } else if (m_value.exchange(v) == old) {
            return;
        }
    }
}


bool planet::telemetry::max::save(serialise::save_buffer &sb) const {
    auto const c = m_value.load();
    if (c != minimum) {
        sb.save_box(box, name(), c);
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::max::load(measurements &pd) {
    value_type c;
    if (load_performance_measurement(pd, name(), box, c)) {
        value(c);
        return true;
    } else {
        return false;
    }
}


namespace {
    auto const max_print = planet::log::format(
            planet::telemetry::max::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                std::uint64_t value;
                box.named(planet::telemetry::max::box, name, value);
                os << name << " = " << value;
            });
}


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


planet::telemetry::performance::performance(
        std::string_view const n, std::source_location const &loc)
: id{n, id::suffix::suppress} {
    std::scoped_lock _{g_mtx};
    auto &perfs = g_perfs();
    auto pos =
            std::lower_bound(perfs.begin(), perfs.end(), n, [](auto p, auto v) {
                return v > p->name();
            });
    if (pos != perfs.end() and (*pos)->name() == n) {
        throw felspar::stdexcept::logic_error{
                "There is already a performance counter called "
                        + std::string{n},
                loc};
    }
    perfs.insert(pos, this);
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
        auto box = serialise::expect_box(lb);
        std::string name;
        serialise::load(box.content, name);
        map.insert(std::pair{std::move(name), std::move(box)});
    }
    return map;
}


/// ## `planet::telemetry::real_time_decay`


namespace {
    double decay_factor(
            std::chrono::nanoseconds const ns, double half_life) noexcept {
        auto const ts = static_cast<double>(ns.count());
        return std::pow(2.0, -ts / half_life);
    }
}


void planet::telemetry::real_time_decay::add_measurement(
        double const m) noexcept {
    auto const decay = decay_factor(last.checkpoint(), half_life);
    auto const a = m * (1 - decay);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * decay + a)) {}
}


bool planet::telemetry::real_time_decay::save(serialise::save_buffer &ab) const {
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


void planet::telemetry::real_time_rate::tick() noexcept {
    std::chrono::nanoseconds ns{last.checkpoint()};
    auto const ts = static_cast<double>(ns.count());
    auto const m = 1e9 / ts;
    auto const rt = std::pow(2.0, -ts / half_life);
    auto const a = m * (1.0 - rt);
    auto ov = m_value.load();
    while (not m_value.compare_exchange_weak(ov, ov * rt + a)) {}
}


bool planet::telemetry::real_time_rate::save(serialise::save_buffer &ab) const {
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
                os << name << " = " << value << "/s";
            });
}


// ## `planet::telemetry::time`


bool planet::telemetry::time::save(serialise::save_buffer &ab) const {
    auto const c = ns.load();
    if (c != 0) {
        ab.save_box(box, name(), c);
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::time::load(measurements &pd) {
    std::int64_t c;
    if (load_performance_measurement(pd, name(), box, c)) {
        ns += c;
        return true;
    } else {
        return false;
    }
}


namespace {
    auto const time_print = planet::log::format(
            planet::telemetry::time::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                std::int64_t count;
                box.named(planet::telemetry::time::box, name, count);
                std::chrono::nanoseconds const ns{count};

                os << name << " = " << std::fixed
                   << static_cast<double>(count / 1e9) << std::defaultfloat
                   << 's';
            });
}


// ## `planet::telemetry::timestamps`


planet::telemetry::timestamps::timestamps(
        std::string_view const n, std::source_location const &loc)
: performance{n, loc} {}


void planet::telemetry::timestamps::set(std::string_view key) {
    auto pos = history.find(key);
    if (pos == history.end()) {
        history.insert(std::pair{std::string{key}, stamps{}});
    } else {
        pos->second.last = std::chrono::system_clock::now();
        ++pos->second.count;
    }
}
void planet::telemetry::timestamps::unset(std::string_view const key) {
    history.erase(history.find(key));
}


bool planet::telemetry::timestamps::is_set(std::string_view const key) const {
    return history.find(key) != history.end();
}


auto planet::telemetry::timestamps::times_for(std::string_view const key) const
        -> std::optional<stamps> {
    auto pos = history.find(key);
    if (pos == history.end()) {
        return {};
    } else {
        return pos->second;
    }
}


bool planet::telemetry::timestamps::save(serialise::save_buffer &sb) const {
    std::scoped_lock _{mutex};
    if (history.size()) {
        sb.save_box(box, name(), history);
        return true;
    } else {
        return false;
    }
}
bool planet::telemetry::timestamps::load(measurements &pd) {
    std::map<std::string, stamps, std::less<>> m;
    if (load_performance_measurement(pd, name(), box, m)) {
        std::scoped_lock _{mutex};
        for (auto const &[k, v] : m) {
            if (auto pos = history.find(k); pos == history.end()) {
                history[k] = v;
            } else {
                pos->second.first = std::min(pos->second.first, v.first);
                pos->second.last = std::max(pos->second.last, v.last);
            }
        }
        return true;
    } else {
        return false;
    }
}


void planet::telemetry::save(
        planet::serialise::save_buffer &sb,
        planet::telemetry::timestamps::stamps const &s) {
    sb.save_box(s.box, s.first, s.last, s.count);
}
void planet::telemetry::load(
        planet::serialise::box &box, planet::telemetry::timestamps::stamps &s) {
    box.lambda(s.box, [&]() {
        box.fields(s.first, s.last);
        if (box.content.empty()) {
            if (s.last) { s.count = 2; }
            return;
        }
        box.fields(s.count);
    });
}

void planet::telemetry::load(planet::serialise::box &b, timestamps &t) {
    std::string id;
    b.named(t.box, id, t.history);
}


namespace {
    std::ostream &
            print(std::ostream &os,
                  planet::telemetry::timestamps::stamps const &s) {
        /**
         * TODO We really want to have a better display of the time stamp,
         * ideally as local time, or UTC or something.
         */
        os << "first = " << s.first.time_since_epoch().count();
        if (s.last) {
            os << " last = " << s.last.value().time_since_epoch().count()
               << " count = " << s.count;
        }
        return os;
    }
    auto const timestamps_print = planet::log::format(
            planet::telemetry::timestamps::box,
            [](std::ostream &os, planet::serialise::box &box) {
                std::string name;
                std::map<
                        std::string, planet::telemetry::timestamps::stamps,
                        std::less<>>
                        timestamps;
                box.named(planet::telemetry::timestamps::box, name, timestamps);
                if (timestamps.size() == 0) {
                    os << name << " (empty)";
                } else if (timestamps.size() == 1) {
                    os << name << ' ' << timestamps.begin()->first << ' ';
                    print(os, timestamps.begin()->second);
                } else {
                    os << name << '\n';
                    for (auto const &[n, s] : timestamps) {
                        os << "    " << n << ' ';
                        print(os, s);
                        os << '\n';
                    }
                }
            });
    auto const timestamps_stamp_print = planet::log::format(
            planet::telemetry::timestamps::stamps::box,
            [](std::ostream &os, planet::serialise::box &box) {
                planet::telemetry::timestamps::stamps s;
                load(box, s);
                print(os, s);
            });
}
