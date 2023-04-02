#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/constrained.hpp>

#include <optional>


namespace planet::ui {


    /// ## Layout element
    template<typename E, typename T = float>
    class element {
      public:
        using value_type = E;
        using constrained_type = constrained2d<T>;

        element() noexcept {}
        explicit element(constrained_type const &s) noexcept : size{s} {}

        constrained_type size;
        std::optional<affine::rectangle2d> position;
        value_type value;
    };


}
