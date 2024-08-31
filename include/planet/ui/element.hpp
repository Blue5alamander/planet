#pragma once


#include <planet/affine/rectangle2d.hpp>
#include <planet/ui/constrained.hpp>
#include <planet/ui/forward.hpp>

#include <optional>


namespace planet::ui {


    /// ## Layout element
    template<typename T>
    struct element<void, T> {
        using constrained_type = constrained2d<T>;


        element() noexcept {}
        explicit element(constrained_type const &s) noexcept : constraints{s} {}


        constrained_type constraints;
        std::optional<affine::rectangle2d> position;
    };


    template<typename E, typename T>
    struct element final : public element<void, T> {
        using element<void, T>::element;

        using value_type = E;
        value_type value;
    };


}
