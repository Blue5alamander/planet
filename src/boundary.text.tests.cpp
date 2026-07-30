#include <planet/text/boundary.hpp>

#include <felspar/test.hpp>

#include <string>


namespace {


    /**
     * The multibyte cases, spelled out so the expected offsets below are
     * checkable by eye: `é` is two bytes, `→` three and `🚀` four.
     */
    constexpr std::string_view e_acute{"é"};
    constexpr std::string_view arrow{"→"};
    constexpr std::string_view rocket{"🚀"};


    auto const suite = felspar::testsuite("text.boundary");


    auto const ascii = suite.test("ascii", [](auto check) {
        constexpr std::string_view text{"abc"};

        check(planet::text::previous_boundary(text, 3)) == 2u;
        check(planet::text::previous_boundary(text, 2)) == 1u;
        check(planet::text::previous_boundary(text, 1)) == 0u;

        check(planet::text::next_boundary(text, 0)) == 1u;
        check(planet::text::next_boundary(text, 1)) == 2u;
        check(planet::text::next_boundary(text, 2)) == 3u;
    });


    auto const multibyte = suite.test(
            "multibyte",
            [](auto check) {
                check(e_acute.size()) == 2u;

                check(planet::text::previous_boundary(e_acute, e_acute.size()))
                        == 0u;
                check(planet::text::next_boundary(e_acute, 0))
                        == e_acute.size();
            },
            [](auto check) {
                check(arrow.size()) == 3u;

                check(planet::text::previous_boundary(arrow, arrow.size()))
                        == 0u;
                check(planet::text::next_boundary(arrow, 0)) == arrow.size();
            },
            [](auto check) {
                check(rocket.size()) == 4u;

                check(planet::text::previous_boundary(rocket, rocket.size()))
                        == 0u;
                check(planet::text::next_boundary(rocket, 0)) == rocket.size();
            },
            [](auto check) {
                /**
                 * Mixed with ASCII either side, so a step lands on the
                 * character boundary rather than on the end of the string.
                 */
                std::string const text = "a" + std::string{arrow} + "b";

                check(planet::text::previous_boundary(text, text.size())) == 4u;
                check(planet::text::previous_boundary(text, 4)) == 1u;
                check(planet::text::previous_boundary(text, 1)) == 0u;

                check(planet::text::next_boundary(text, 0)) == 1u;
                check(planet::text::next_boundary(text, 1)) == 4u;
                check(planet::text::next_boundary(text, 4)) == 5u;
            });


    auto const ends = suite.test("ends", [](auto check) {
        constexpr std::string_view text{"abc"};

        /// Neither function runs off the end of the string it was given
        check(planet::text::previous_boundary(text, 0)) == 0u;
        check(planet::text::next_boundary(text, text.size())) == text.size();

        /// Nor past it, for an offset that was never legal in the first place
        check(planet::text::previous_boundary(text, 99)) == 2u;
        check(planet::text::next_boundary(text, 99)) == text.size();
    });


    auto const mid_character = suite.test("mid character", [](auto check) {
        /**
         * An offset that splits a character still comes back as a boundary,
         * which is what lets a cursor set from a measurement be snapped onto
         * one.
         */
        std::string const text = "a" + std::string{rocket};

        check(planet::text::previous_boundary(text, 3)) == 1u;
        check(planet::text::next_boundary(text, 3)) == text.size();

        check(planet::text::previous_boundary(text, 2)) == 1u;
        check(planet::text::next_boundary(text, 2)) == text.size();
    });


    auto const empty = suite.test("empty", [](auto check) {
        constexpr std::string_view text{};

        check(planet::text::previous_boundary(text, 0)) == 0u;
        check(planet::text::previous_boundary(text, 7)) == 0u;
        check(planet::text::next_boundary(text, 0)) == 0u;
        check(planet::text::next_boundary(text, 7)) == 0u;
    });


}
