#include <planet/platform.hpp>
#include <planet/serialise.hpp>
#include <planet/version.hpp>

#include <felspar/exceptions/runtime_error.hpp>
#include <felspar/test.hpp>


#ifdef __linux__
static_assert(planet::current_platform == planet::platform::linux);
#endif


namespace {


    auto const suite = felspar::testsuite(
            "platform",
            [](auto check) {
                for (auto const p :
                     {planet::platform::unknown, planet::platform::linux,
                      planet::platform::windows, planet::platform::macos,
                      planet::platform::android, planet::platform::ios}) {
                    check(planet::platform_from_string(planet::to_string(p)))
                            == p;
                }
                check([]() {
                    planet::platform_from_string("not-a-platform");
                }).template throws_type<felspar::stdexcept::runtime_error>();
            },
            [](auto check) {
                planet::serialise::save_buffer sb;
                save(sb, planet::platform::ios);
                auto const bytes{sb.complete()};
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                check(planet::serialise::load_type<planet::platform>(lb))
                        == planet::platform::ios;
            },
            [](auto check) {
                planet::version const v{"appid", "1.2.3", 4};
                check(v.platform) == planet::current_platform;

                planet::serialise::save_buffer sb;
                save(sb, v);
                auto const bytes{sb.complete()};
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                auto b = planet::serialise::expect_box(lb);
                planet::version const loaded{b};
                check(loaded.version_string) == "1.2.3";
                check(loaded.platform) == planet::current_platform;
            },
            [](auto check) {
                /**
                 * A version saved before the `platform` field existed must
                 * still load, with the platform falling back to `unknown`.
                 */
                planet::serialise::save_buffer sb;
                sb.save_box(
                        planet::version::box, std::string{"appid"},
                        std::string{"1.2.3"}, planet::semver{1, 2, 3},
                        std::optional<std::uint16_t>{4});
                auto const bytes{sb.complete()};
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                auto b = planet::serialise::expect_box(lb);
                planet::version const loaded{b};
                check(loaded.version_string) == "1.2.3";
                check(loaded.git_describe) == "";
                check(loaded.platform) == planet::platform::unknown;
            });


}
