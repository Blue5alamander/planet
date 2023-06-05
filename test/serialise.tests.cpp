#include <planet/serialise.hpp>
#include <felspar/test.hpp>

#include <felspar/memory/hexdump.hpp>


static_assert(
        planet::serialise::marker_for<bool>() == planet::serialise::marker::u8);
static_assert(
        planet::serialise::marker_for<std::int8_t>()
        == planet::serialise::marker::i8);


namespace {


    auto const suite = felspar::testsuite("serialize");


    auto const e = suite.test("empty", [](auto check, auto &log) {
        planet::serialise::save_buffer ab;
        auto const bytes{ab.save_box("empty").complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 15u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 5u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'e';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'm';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'p';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 't';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'y';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 1u;
        check(felspar::parse::binary::native::extract<std::int64_t>(span))
                == 0u;
        check(span.empty()) == true;
    });


    struct small {
        std::uint32_t field1 = {};
    };
    /**
     * Although the `save` functions used by the serialisation generally return
     * `void`, they can return anything you want. The tests return the
     * `save_buffer` because it means that the tests can call `save` and then
     * `complete()` the returned value. This just serves to make the test code a
     * little shorter.
     */
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, small const &s) {
        return ab.save_box("small", s.field1);
    }
    void load(planet::serialise::load_buffer &lb, small &s) {
        lb.load_box("small", s.field1);
    }
    auto const s = suite.test("small", [](auto check, auto &log) {
        small const s{1234};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, s).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 20u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 5u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 's';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'm';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'a';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'l';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'l';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 1u;
        check(felspar::parse::binary::be::extract<std::int64_t>(span)) == 5u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == static_cast<std::uint8_t>(planet::serialise::marker::u32le);
        check(felspar::parse::binary::native::extract<std::uint32_t>(span))
                == 1234u;
        check(span.empty()) == true;

        auto const ss = planet::serialise::load_type<small>(bytes);
        check(ss.field1) == 1234u;
        felspar::memory::hexdump(log, bytes.cmemory());
    });


    struct larger {
        bool field1 = {};
        std::int32_t field2 = {};
        std::int16_t field3 = {};
        std::uint64_t field4 = {};
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, larger const &l) {
        return ab.save_box("larger", l.field1, l.field2, l.field3, l.field4);
    }
    void load(planet::serialise::load_buffer &lb, larger &l) {
        lb.load_box("larger", l.field1, l.field2, l.field3, l.field4);
    }
    auto const l = suite.test("larger", [](auto check, auto &log) {
        larger const l{true, -0x12345678, 0x1234, 0x12345678'90987654};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, l).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 34u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 6u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'l';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'a';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'r';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'g';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'e';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'r';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 1u;
        check(felspar::parse::binary::be::extract<std::uint64_t>(span)) == 18u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == static_cast<std::uint8_t>(planet::serialise::marker::b_true);
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == static_cast<std::uint8_t>(planet::serialise::marker::i32le);
        check(felspar::parse::binary::native::extract<std::int32_t>(span))
                == -0x12345678;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == static_cast<std::uint8_t>(planet::serialise::marker::i16le);
        check(felspar::parse::binary::native::extract<std::int16_t>(span))
                == 0x1234;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == static_cast<std::uint8_t>(planet::serialise::marker::u64le);
        check(felspar::parse::binary::native::extract<std::uint64_t>(span))
                == 0x12345678'90987654U;
        check(span.empty()) == true;

        auto const ll = planet::serialise::load_type<larger>(bytes);
        check(ll.field1) == true;
        check(ll.field2) == -0x12345678;
        check(ll.field3) == 0x1234;
        check(ll.field4) == 0x12345678'90987654U;
    });


    struct nested {
        small s;
        larger l;
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, nested const &n) {
        return ab.save_box("nested", n.s, n.l);
    }
    void load(planet::serialise::load_buffer &lb, nested &n) {
        lb.load_box("nested", n.s, n.l);
    }
    auto const n = suite.test("nested", [](auto check, auto &log) {
        nested const n{
                {0x1234}, {false, -0x12345678, 0x1234, 0x12345678'90987654}};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, n).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 70u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 6u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'n';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'e';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 's';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 't';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'e';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'd';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 1u;
        check(felspar::parse::binary::be::extract<std::int64_t>(span)) == 54u;
        check(span.size()) == 54u;

        auto const nn = planet::serialise::load_type<nested>(bytes);
        check(nn.s.field1) == 0x1234u;
        check(nn.l.field1) == false;
        check(nn.l.field2) == -0x12345678;
        check(nn.l.field3) == 0x1234;
        check(nn.l.field4) == 0x12345678'90987654U;
    });


    auto const m = suite.test(
            "square map",
            [](auto check, auto &log) {
                planet::map::world_type<bool, 2> world{
                        {0, 0}, [](auto) { return true; }};

                planet::serialise::save_buffer ab;
                save(ab, world);
                auto bytes{ab.complete()};
                felspar::memory::hexdump(log, bytes.memory());
                check(bytes.size()) == 29u;

                auto span = bytes.cmemory();
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 10u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == '_';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'p';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'm';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'w';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'o';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'r';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'l';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'd';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 1u;
                check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                        == 9u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 0xa0;
                check(felspar::parse::binary::native::extract<std::uint64_t>(
                        span))
                        == 0u;
                check(span.empty()) == true;

                planet::map::world_type<bool, 2> nw{
                        {{}, {}}, [](auto) { return true; }};
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                load(lb, nw);
                check(nw[{0, 0}]) == true;
            },
            [](auto check, auto &log) {
                planet::map::world_type<bool, 2> world{
                        {0, 0}, [](auto) { return true; }};

                world[{0, 0}] = false;
                planet::serialise::save_buffer ab;
                save(ab, world);
                auto bytes{ab.complete()};
                felspar::memory::hexdump(log, bytes.memory());
                check(bytes.size()) == 92u;

                auto span = bytes.cmemory();
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 10u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == '_';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'p';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'm';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'w';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'o';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'r';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'l';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'd';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 1u;
                check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                        == 72u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 0xa0;
                check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                        == 2u;
                felspar::memory::hexdump(log, span);

                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 10u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == '_';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'p';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'm';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'c';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'o';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'o';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'r';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'd';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 1u;
                check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                        == 10u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::i32le);
                check(felspar::parse::binary::native::extract<std::int32_t>(
                        span))
                        == 0;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::i32le);
                check(felspar::parse::binary::native::extract<std::int32_t>(
                        span))
                        == 0;
                felspar::memory::hexdump(log, span);

                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 10u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == '_';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'p';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'm';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == ':';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'c';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'h';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'u';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'n';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 'k';
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 1u;
                check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                        == 13u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == 0xa0;
                check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                        == 4u;
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::b_false);
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::b_true);
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::b_true);
                check(felspar::parse::binary::native::extract<std::uint8_t>(
                        span))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::b_true);
                check(span.empty()) == true;

                planet::map::world_type<bool, 2> nw{
                        {{}, {}}, [](auto) { return true; }};
                auto lb = planet::serialise::load_buffer{bytes.cmemory()};
                load(lb, nw);
                check(nw[{0, 0}]) == false;
            });


    struct binary {
        std::vector<std::byte> vec;
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, binary const &b) {
        return ab.save_box("bin", b.vec);
    }
    void load(planet::serialise::load_buffer &lb, binary &b) {
        lb.load_box("bin", b.vec);
    }
    auto const b = suite.test("binary", [](auto check, auto &log) {
        binary const b{std::vector<std::byte>(20 << 10, std::byte{'-'})};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, b).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 20480u + 3u + 16u + 2u + 1u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 3u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'b';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'i';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 'n';
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == 1u;
        check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                == 20489u;
        check(felspar::parse::binary::native::extract<std::uint8_t>(span))
                == static_cast<std::uint8_t>(
                        planet::serialise::marker::std_byte_array);
        check(felspar::parse::binary::be::extract<std::uint64_t>(span))
                == 20480u;
        check(span.size()) == 20480u;

        binary bb{};
        auto lb = planet::serialise::load_buffer{bytes.cmemory()};
        load(lb, bb);
        check(bb.vec.size()) == 20480u;
        check(std::count(bb.vec.begin(), bb.vec.end(), std::byte{'-'}))
                == 20480u;
    });


    auto const st = suite.test("std", [](auto check, auto &log) {
        planet::serialise::save_buffer ab;

        auto const ts = std::chrono::system_clock::now();
        ab.save_box("std", ts);
        auto bytes{ab.complete()};
        felspar::memory::hexdump(log, bytes.memory());

        auto lb = planet::serialise::load_buffer{bytes.cmemory()};
        std::chrono::system_clock::time_point tp;
        lb.load_box("std", tp);
        check(tp) == ts;
    });


    auto const af = suite.test("affine", [](auto check, auto &log) {
        planet::serialise::save_buffer ab;

        auto p = planet::affine::point2d{3.5f, 4.25f};
        save(ab, p);

        auto bytes{ab.complete()};
        felspar::memory::hexdump(log, bytes.memory());
        auto lb = planet::serialise::load_buffer{bytes.cmemory()};

        check(load_type<planet::affine::point2d>(lb)) == p;
    });


    auto const end = suite.test(
            "endian",
            [](auto check) {
                std::array<std::uint8_t, 3> ar{0xa3, 0x34, 0x12};
                planet::serialise::load_buffer lb{std::as_bytes(std::span{ar})};
                std::uint16_t u16;
                planet::serialise::load(lb, u16);
                check(u16) == 0x1234u;
            },
            [](auto check) {
                std::array<std::uint8_t, 3> ar{0x83, 0x12, 0x34};
                planet::serialise::load_buffer lb{std::as_bytes(std::span{ar})};
                std::uint16_t u16;
                planet::serialise::load(lb, u16);
                check(u16) == 0x1234u;
            });


}
