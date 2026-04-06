#include <planet/serialise.hpp>
#include <planet/telemetry/allocator.strategy.hpp>
#include <planet/telemetry/duration.hpp>
#include <planet/telemetry/map.hpp>
#include <planet/telemetry/timestamps.hpp>

#include <felspar/memory/slab.storage.hpp>
#include <felspar/memory/hexdump.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite_steady_duration = felspar::testsuite(
            "steady_duration",
            [](auto check) {
                using namespace std::chrono_literals;
                planet::telemetry::steady_duration d{"test_sd_single", 16};
                d.add_measurement(1ms);
                check(d.value().count()) > 0;
            },
            [](auto check) {
                using namespace std::chrono_literals;
                planet::telemetry::steady_duration d{"test_sd_converge", 4};
                for (int i = 0; i < 100; ++i) { d.add_measurement(1ms); }
                // After many readings with half-life=4, value should be close
                // to 1ms (within 10%)
                auto const v = d.value();
                check(v.count()) >= 900'000;
                check(v.count()) <= 1'100'000;
            },
            [](auto check) {
                using namespace std::chrono_literals;
                auto bytes = []() {
                    planet::telemetry::steady_duration d{"test_sd_save", 8};
                    d.add_measurement(2ms);
                    planet::serialise::save_buffer sb;
                    planet::telemetry::save_performance(sb, d);
                    return sb.complete();
                }();
                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::steady_duration d2{"test_sd_save", 8};
                planet::telemetry::load_performance(lb, d2);
                check(d2.value().count()) > 0;
            });


    auto const suite =
            felspar::testsuite("timestamps", [](auto check, auto &log) {
                auto [bytes, first, second] = [&]() {
                    planet::telemetry::timestamps times{"test1"};

                    times.set("first");
                    check(times.is_set("first")) == true;
                    check(times.is_set("second")) == false;
                    times.set("second");
                    check(times.is_set("second")) == true;
                    times.unset("first");
                    check(times.is_set("first")) == false;
                    check(times.is_set("second")) == true;

                    planet::serialise::save_buffer sb;
                    planet::telemetry::save_performance(sb, times);
                    return std::tuple{
                            sb.complete(), times.times_for("first"),
                            times.times_for("second")};
                }();
                log << felspar::memory::hexdump(bytes.cmemory());

                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::timestamps ts{"test1"};
                planet::telemetry::load_performance(lb, ts);
                check(ts.is_set("first")) == false;
                check(ts.is_set("second")) == true;

                check(ts.times_for("first")) == first;
                check(ts.times_for("second")) == second;
            });


    auto const suite_map = felspar::testsuite(
            "map",
            [](auto check) {
                planet::telemetry::map<std::size_t, std::size_t> m{"test_map"};
                check(m.size()) == 0u;

                /// Update new key inserts initial value
                bool lambda_called = false;
                m.update(10, 1, [&](auto &) { lambda_called = true; });
                check(lambda_called) == false;
                check(m.size()) == 1u;
                auto content = m.copy_content();
                check(content.size()) == 1u;
                check(content[10]) == 1u;

                /// Update existing key calls lambda
                lambda_called = false;
                std::size_t old_value = 0;
                m.update(10, 999, [&](auto &v) {
                    old_value = v;
                    ++v;
                    lambda_called = true;
                });
                check(lambda_called) == true;
                check(old_value) == 1u;
                check(m.size()) == 1u;
                content = m.copy_content();
                check(content.size()) == 1u;
                check(content[10]) == 2u;

                /// Update with different key
                m.update(20, 1, [](auto &) {});
                check(m.size()) == 2u;
                content = m.copy_content();
                check(content.size()) == 2u;
                check(content[10]) == 2u;
                check(content[20]) == 1u;
            },
            [](auto check, auto &log) {
                auto bytes = [&]() {
                    planet::telemetry::map<std::size_t, std::size_t> m{
                            "test_map_save"};
                    m.update(10, 1, [](auto &) {});
                    m.update(10, 1, [](auto &n) { ++n; });
                    m.update(10, 1, [](auto &n) { ++n; });
                    m.update(20, 1, [](auto &) {});

                    auto content = m.copy_content();
                    check(content.size()) == 2u;
                    check(content[10]) == 3u;
                    check(content[20]) == 1u;

                    planet::serialise::save_buffer sb;
                    planet::telemetry::save_performance(sb, m);
                    return sb.complete();
                }();
                log << felspar::memory::hexdump(bytes.cmemory());

                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::map<std::size_t, std::size_t> m2{
                        "test_map_save"};
                planet::telemetry::load_performance(lb, m2);
                auto loaded_content = m2.copy_content();
                check(loaded_content.size()) == 2u;
                check(loaded_content[10]) == 3u;
                check(loaded_content[20]) == 1u;
            });


    auto const suite_allocator_strategy = felspar::testsuite(
            "allocator",
            [](auto check) {
                planet::telemetry::allocator_strategy<
                        felspar::memory::slab_storage<1024>>
                        telemetry_slab{"test_allocator"};

                /// Allocate the same size three times and a different size once
                constexpr std::size_t size_a = 16;
                constexpr std::size_t size_b = 32;
                [[maybe_unused]] auto p1 = telemetry_slab.allocate(size_a);
                [[maybe_unused]] auto p2 = telemetry_slab.allocate(size_a);
                [[maybe_unused]] auto p3 = telemetry_slab.allocate(size_a);
                [[maybe_unused]] auto p4 = telemetry_slab.allocate(size_b);

                /// Verify copy_content returns the expected histogram
                auto content = telemetry_slab.telemetry.copy_content();
                check(content.size()) == 2u;
                check(content[size_a]) == 3u;
                check(content[size_b]) == 1u;
            },
            [](auto check, auto &log) {
                /// Verify telemetry data round-trips through save/load
                auto bytes = []() {
                    planet::telemetry::allocator_strategy<
                            felspar::memory::slab_storage<1024>>
                            telemetry_slab{"test_allocator_save"};

                    [[maybe_unused]] auto p1 = telemetry_slab.allocate(16);
                    [[maybe_unused]] auto p2 = telemetry_slab.allocate(16);
                    [[maybe_unused]] auto p3 = telemetry_slab.allocate(16);
                    [[maybe_unused]] auto p4 = telemetry_slab.allocate(32);

                    planet::serialise::save_buffer sb;
                    planet::telemetry::save_performance(
                            sb, telemetry_slab.telemetry);
                    return sb.complete();
                }();
                log << felspar::memory::hexdump(bytes.cmemory());

                planet::serialise::load_buffer lb{bytes};
                planet::telemetry::allocator_strategy<
                        felspar::memory::slab_storage<1024>>
                        telemetry_slab2{"test_allocator_save"};
                planet::telemetry::load_performance(
                        lb, telemetry_slab2.telemetry);

                auto loaded_content = telemetry_slab2.telemetry.copy_content();
                check(loaded_content.size()) == 2u;
                check(loaded_content[16]) == 3u;
                check(loaded_content[32]) == 1u;
            });


}
