#pragma once


#include <planet/serialise/forward.hpp>


namespace planet {


    /// ## Linear colour format
    /**
     * The colours described by this time are intended for use as a linear
     * colour space, so colour manipulation is a simple matter of multiplying
     * the values.
     */
    struct colour {
        static constexpr std::string_view box{"_p:col"};


        float r{}, g{}, b{}, a = {1};


        /// ### New colours via mutation
        colour with_alpha(float const na) const noexcept {
            return {r, g, b, na};
        }


        /// ### Fixed colours
        static colour const black, white;


        /// ### Multiply the RGB values
        friend colour operator*(colour const &c, float const m) noexcept {
            return {c.r * m, c.g * m, c.b * m, c.a};
        }
    };
    void save(planet::serialise::save_buffer &, colour const &);
    void load(planet::serialise::box &, colour &);


    inline constexpr colour colour::black{};
    inline constexpr colour colour::white{1.0f, 1.0f, 1.0f};


}
