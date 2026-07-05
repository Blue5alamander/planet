#include <planet/serialise.hpp>
#include <felspar/test.hpp>


/**
 * # Serialising `std::variant`
 *
 * This file is documentation first and tests second. It shows how to round-trip
 * a `std::variant` through the serialisation layer in the scenarios that come
 * up in practice, and the tests exist to prove the techniques shown here
 * actually work.
 *
 * The save side is generic and lives in
 * [`planet/serialise/variant.hpp`](../include/planet/serialise/variant.hpp):
 * the active alternative is written out directly, with **no enclosing box of
 * its own**. Saving a `std::variant<Ts...>` therefore produces exactly the
 * bytes that saving the held alternative would.
 *
 * Because there is no wrapping box, the load side recovers which alternative
 * was written from the leading byte of the data — the `marker`:
 *
 * 1. A box marker (`0x01`–`0x7f`) means the alternative is a boxed type, and
 * the box name selects it.
 * 2. Any other marker identifies a primitive alternative via `marker_for<T>()`.
 *
 * The only requirement is that the alternatives have distinct discriminators: a
 * unique box name per boxed alternative and a unique marker per primitive one.
 *
 * ## A caveat on argument-dependent lookup
 *
 * `command` (below) is only an alias for `std::variant<...>`, so an unqualified
 * `save`/`load` call finds overloads by ADL using the *underlying* type's
 * associated namespaces — `std` plus the namespaces of the alternative types —
 * not wherever the alias happens to be declared. The hand-written `load`
 * overloads here are found only because the alternatives live in this same
 * namespace, making it an associated namespace.
 *
 * When the alternatives' namespaces cannot host the loader — they are all from
 * `std`, or from a library you should not add overloads to — declare it in
 * `planet::serialise` instead, as scenario 4 shows. The `load_buffer` argument
 * makes `planet::serialise` an associated namespace of every `load` call, so
 * an overload there is found from anywhere — including the field folds inside
 * `named` and `fields`, which call `load` from inside the machinery where a
 * `using` declaration at your call site cannot help. For calls you write
 * yourself a `using` declaration (or a qualified call) also works.
 */


namespace serialise_variant_documentation {


    auto const suite = felspar::testsuite("serialise/variant");


    /**
     * ## Scenario 1 — a variant of boxed types
     *
     * The common case: every alternative is a user type that serialises as a
     * named box. Each one gets the usual pair of free functions — a `save` that
     * writes a box, and a `load` that reads it back from a `box`. Exposing the
     * box name as a `static constexpr` member lets both the type and the
     * variant loader refer to the same string.
     */

    struct teleport {
        static constexpr std::string_view box{"doc::variant::teleport"};
        std::int32_t x = {}, y = {};
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, teleport const &t) {
        return ab.save_box(teleport::box, t.x, t.y);
    }
    void load(planet::serialise::box &b, teleport &t) {
        b.named(teleport::box, t.x, t.y);
    }

    struct wait {
        static constexpr std::string_view box{"doc::variant::wait"};
        std::int64_t turns = {};
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, wait const &w) {
        return ab.save_box(wait::box, w.turns);
    }
    void load(planet::serialise::box &b, wait &w) {
        b.named(wait::box, w.turns);
    }

    using command = std::variant<teleport, wait>;

    /**
     * Saving a `command` needs nothing extra — the generic variant `save`
     * visits the active alternative and delegates to its `save`. The load is
     * written with the ordinary loader signature, `load(load_buffer &, command
     * &)`: pull the box out of the buffer, then dispatch on its name and
     * `emplace` the matching alternative for its own `load` to fill in.
     *
     * Because it has the standard shape, a `command` can be listed as a field
     * in any other type's loader (`b.named("...", first, command_field, last)`)
     * just like any other serialisable member.
     */
    void load(planet::serialise::load_buffer &lb, command &c) {
        auto b = planet::serialise::expect_box(lb);
        if (b.name == teleport::box) {
            load(b, c.emplace<teleport>());
        } else if (b.name == wait::box) {
            load(b, c.emplace<wait>());
        } else {
            throw felspar::stdexcept::runtime_error{"Unknown command box name"};
        }
    }

    auto const boxed = suite.test("boxed alternatives", [](auto check, auto &) {
        for (command const original :
             {command{teleport{3, -4}}, command{wait{7}}}) {
            planet::serialise::save_buffer ab;
            save(ab, original);
            auto const bytes = ab.complete();

            planet::serialise::load_buffer lb{bytes.cmemory()};
            command restored;
            load(lb, restored);
            lb.check_empty_or_throw();

            check(restored.index()) == original.index();
        }

        command const original{teleport{3, -4}};
        planet::serialise::save_buffer ab;
        save(ab, original);
        planet::serialise::load_buffer lb{ab.complete().cmemory()};
        command restored;
        load(lb, restored);
        check(std::get<teleport>(restored).x) == 3;
        check(std::get<teleport>(restored).y) == -4;
    });


    /**
     * ## Scenario 2 — a variant mixing a primitive and a boxed type
     *
     * When an alternative is a primitive it is written as a bare marker, not a
     * box, so `expect_box` would reject it. Peek the leading marker first: box
     * markers go down the named-box path from scenario 1, everything else is a
     * primitive that loads directly. The marker partitioning guarantees the two
     * paths never overlap.
     */

    struct named_ref {
        static constexpr std::string_view box{"doc::variant::named_ref"};
        std::string name;
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, named_ref const &r) {
        return ab.save_box(named_ref::box, r.name);
    }
    void load(planet::serialise::box &b, named_ref &r) {
        b.named(named_ref::box, r.name);
    }

    using value = std::variant<std::int64_t, named_ref>;

    void load(planet::serialise::load_buffer &lb, value &v) {
        if (planet::serialise::is_box_marker(lb.peek_marker())) {
            auto b = planet::serialise::expect_box(lb);
            if (b.name == named_ref::box) {
                load(b, v.emplace<named_ref>());
            } else {
                throw felspar::stdexcept::runtime_error{
                        "Unknown value box name"};
            }
        } else {
            load(lb, v.emplace<std::int64_t>());
        }
    }

    auto const mixed = suite.test("mixed alternatives", [](auto check, auto &) {
        {
            value const original{std::int64_t{99}};
            planet::serialise::save_buffer ab;
            save(ab, original);
            planet::serialise::load_buffer lb{ab.complete().cmemory()};
            value restored;
            load(lb, restored);
            lb.check_empty_or_throw();
            check(std::get<std::int64_t>(restored)) == 99;
        }
        {
            value const original{named_ref{"alpha"}};
            planet::serialise::save_buffer ab;
            save(ab, original);
            planet::serialise::load_buffer lb{ab.complete().cmemory()};
            value restored;
            load(lb, restored);
            lb.check_empty_or_throw();
            check(std::get<named_ref>(restored).name) == std::string{"alpha"};
        }
    });


    /**
     * ## Scenario 3 — `std::monostate` as an empty alternative
     *
     * `std::monostate` is the usual way to give a variant a "nothing here"
     * state and to make it default constructible. It carries no data, so it
     * serialises as a single `marker::empty` byte (`0x00`). Its `save`/`load`
     * are provided by `planet/serialise/variant.hpp`, so an empty alternative
     * needs no extra code from the caller.
     *
     * The loader peeks the marker exactly as scenario 2 does, but each
     * alternative is matched explicitly: a box marker selects `named_ref`, the
     * `empty` marker selects `std::monostate`, and anything else is an error.
     */

    using maybe = std::variant<std::monostate, named_ref>;

    void load(planet::serialise::load_buffer &lb, maybe &m) {
        auto const lead = lb.peek_marker();
        if (planet::serialise::is_box_marker(lead)) {
            auto b = planet::serialise::expect_box(lb);
            if (b.name == named_ref::box) {
                load(b, m.emplace<named_ref>());
            } else {
                throw felspar::stdexcept::runtime_error{
                        "Unknown maybe box name"};
            }
        } else if (lead == planet::serialise::marker::empty) {
            load(lb, m.emplace<std::monostate>());
        } else {
            throw felspar::stdexcept::runtime_error{"Unknown maybe marker"};
        }
    }

    auto const empty =
            suite.test("monostate alternative", [](auto check, auto &) {
                maybe const original{std::monostate{}};
                planet::serialise::save_buffer ab;
                save(ab, original);
                auto const bytes = ab.complete();

                check(bytes.size()) == 1u;
                check(static_cast<std::uint8_t>(bytes.cmemory().front()))
                        == static_cast<std::uint8_t>(
                                planet::serialise::marker::empty);

                planet::serialise::load_buffer lb{bytes.cmemory()};
                maybe restored{named_ref{"not empty"}};
                load(lb, restored);
                lb.check_empty_or_throw();
                check(std::holds_alternative<std::monostate>(restored)) == true;
            });


    /**
     * ## Scenario 4 — alternatives from namespaces you cannot extend
     *
     * Here every alternative comes from `std`, so there is no namespace of
     * your own that ADL will associate with the variant, and adding overloads
     * to `std` is not allowed. A library type you shouldn't write into the
     * namespace of — a `std::variant` of planet types, say — leaves you in the
     * same position. This is also where a `using` declaration falls short: it
     * only fixes up calls you write yourself, and the field folds inside
     * `named` and `fields` call `load` from inside `planet::serialise`, out of
     * its reach.
     *
     * Declare the loader in `planet::serialise` instead. The `load_buffer`
     * argument makes `planet::serialise` an associated namespace of every
     * `load` call, so the overload is found from anywhere — your own code and
     * the machinery alike.
     */

    using setting = std::variant<std::int64_t, std::string>;

}

namespace planet::serialise {

    /**
     * Both alternatives are primitives, so the loader dispatches on the
     * leading marker just as scenario 2 does: `marker_for` names the integer's
     * marker and a string leads with the marker for its character size.
     */
    void load(load_buffer &lb, serialise_variant_documentation::setting &s) {
        auto const lead = lb.peek_marker();
        if (lead == marker_for<std::int64_t>()) {
            load(lb, s.emplace<std::int64_t>());
        } else if (lead == marker::u8string8) {
            load(lb, s.emplace<std::string>());
        } else {
            throw felspar::stdexcept::runtime_error{"Unknown setting marker"};
        }
    }

}

namespace serialise_variant_documentation {

    /**
     * The `setting` field here is saved and loaded by `save_box` and `named`'s
     * field folds — the calls from inside the machinery that only the
     * `planet::serialise` placement can satisfy.
     */
    struct prefs {
        static constexpr std::string_view box{"doc::variant::prefs"};
        setting value;
    };
    planet::serialise::save_buffer &
            save(planet::serialise::save_buffer &ab, prefs const &p) {
        return ab.save_box(prefs::box, p.value);
    }
    void load(planet::serialise::box &b, prefs &p) {
        b.named(prefs::box, p.value);
    }

    auto const elsewhere = suite.test(
            "loader in planet::serialise", [](auto check, auto &) {
                {
                    prefs const original{.value = std::int64_t{42}};
                    planet::serialise::save_buffer ab;
                    save(ab, original);
                    planet::serialise::load_buffer lb{ab.complete().cmemory()};
                    prefs restored;
                    load(lb, restored);
                    lb.check_empty_or_throw();
                    check(std::get<std::int64_t>(restored.value)) == 42;
                }
                {
                    prefs const original{.value = std::string{"blue"}};
                    planet::serialise::save_buffer ab;
                    save(ab, original);
                    planet::serialise::load_buffer lb{ab.complete().cmemory()};
                    prefs restored;
                    load(lb, restored);
                    lb.check_empty_or_throw();
                    check(std::get<std::string>(restored.value))
                            == std::string{"blue"};
                }
            });


}
