#pragma once


#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>
#include <planet/variant.hpp>


namespace planet::serialise {


    /// ## `std::variant` serialisation
    /**
     * The active alternative is serialised directly, **without** an enclosing
     * box of its own. The variant's serialised form is therefore exactly the
     * serialised form of whichever alternative is currently held — saving a
     * `std::variant<Ts...>` produces the same bytes as saving the active
     * `T` would on its own.
     *
     * This works because the load side can recover the discriminator from the
     * leading marker of the saved data (see `marker.hpp`):
     *
     * 1. A box marker (`0x01`–`0x7f`) means the alternative is a boxed type;
     * the box name selects the alternative.
     * 2. Any other marker (`0x80`+) identifies a primitive alternative directly
     *    via `marker_for<T>()`.
     *
     * Because there is no outer box, the alternatives must have pairwise
     * distinct discriminators: a unique marker for each primitive alternative,
     * and a unique box name for each boxed alternative. Two alternatives that
     * serialise the same way (e.g. `std::optional<A>` and `std::optional<B>`,
     * which both box as `"_s:opt"`, or `std::vector<A>` and `std::vector<B>`,
     * which both use the `poly_list` marker) cannot be disambiguated.
     *
     * See the [variant tests](../../../src/serialise.variant.tests.cpp) for
     * worked examples covering both boxed and primitive alternatives.
     */
    template<typename... Ts>
    void save(save_buffer &ab, std::variant<Ts...> const &v) {
        planet::visit(
                v, [&ab](auto const &alternative) { save(ab, alternative); });
    }


    /// ## `std::monostate`
    /**
     * Used as a variant alternative to mean "nothing here". It carries no data,
     * so it serialises as a single `marker::empty` byte. These overloads live
     * in `planet::serialise` (rather than next to a user type) because
     * `monostate` belongs to `std`, and ADL on a `std::variant` alternative
     * only considers the alternative's own associated namespaces.
     */
    inline void save(save_buffer &ab, std::monostate const) {
        ab.append(marker::empty);
    }
    inline void load(load_buffer &lb, std::monostate &) {
        lb.check_marker(marker::empty);
    }


}
