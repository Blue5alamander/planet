#include <planet/serialise.hpp>
#include <felspar/test.hpp>

#include <felspar/memory/hexdump.hpp>


namespace {


    auto const suite = felspar::testsuite("serialize");


    auto const e = suite.test("empty", [](auto check, auto &log) {
        planet::serialise::save_buffer ab;
        auto const bytes{ab.save_box("empty").complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 22u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::extract<std::size_t>(span)) == 5u;
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'e';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'm';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'p';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 't';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'y';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 1u;
        check(felspar::parse::binary::extract<std::size_t>(span)) == 0u;
        check(span.empty()) == true;
    });


    struct small {
        std::uint32_t field1 = {};
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, small const &s) {
        return ab.save_box("small", s.field1);
    }
    void load(planet::serialise::load_buffer &b, small &s) {
        planet::serialise::load_box(b, "small", s.field1);
    }
    auto const s = suite.test("small", [](auto check, auto &log) {
        small const s{1234};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, s).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 26u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::extract<std::size_t>(span)) == 5u;
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 's';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'm';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'a';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'l';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'l';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 1u;
        check(felspar::parse::binary::extract<std::size_t>(span)) == 4u;
        check(felspar::parse::binary::extract<std::uint32_t>(span)) == 1234u;
        check(span.empty()) == true;

        auto const ss = planet::serialise::load_type<small>(bytes);
        check(ss.field1) == 1234u;
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
    void load(planet::serialise::load_buffer &b, larger &l) {
        planet::serialise::load_box(
                b, "larger", l.field1, l.field2, l.field3, l.field4);
    }
    auto const l = suite.test("larger", [](auto check, auto &log) {
        larger const l{true, -0x12345678, 0x1234, 0x12345678'90987654};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, l).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 38u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::extract<std::size_t>(span)) == 6u;
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'l';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'a';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'r';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'g';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'e';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'r';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 1u;
        check(felspar::parse::binary::extract<std::size_t>(span)) == 15u;
        check(felspar::parse::binary::extract<bool>(span)) == true;
        check(felspar::parse::binary::extract<std::int32_t>(span))
                == -0x12345678;
        check(felspar::parse::binary::extract<std::int16_t>(span)) == 0x1234;
        check(felspar::parse::binary::extract<std::int64_t>(span))
                == 0x12345678'90987654;
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
    void load(planet::serialise::load_buffer &b, nested &n) {
        planet::serialise::load_box(b, "nested", n.s, n.l);
    }
    auto const n = suite.test("nested", [](auto check, auto &log) {
        nested const n{
                {0x1234}, {true, -0x12345678, 0x1234, 0x12345678'90987654}};
        planet::serialise::save_buffer ab;

        auto bytes{save(ab, n).complete()};
        felspar::memory::hexdump(log, bytes.memory());
        check(bytes.size()) == 87u;

        auto span = bytes.cmemory();
        check(felspar::parse::binary::extract<std::size_t>(span)) == 6u;
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'n';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'e';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 's';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 't';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'e';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 'd';
        check(felspar::parse::binary::extract<std::uint8_t>(span)) == 1u;
        check(felspar::parse::binary::extract<std::size_t>(span)) == 64u;
        check(span.size()) == 64u;

        auto const nn = planet::serialise::load_type<nested>(bytes);
        check(nn.s.field1) == 0x1234u;
        check(nn.l.field1) == true;
        check(nn.l.field2) == -0x12345678;
        check(nn.l.field3) == 0x1234;
        check(nn.l.field4) == 0x12345678'90987654U;
    });


}
