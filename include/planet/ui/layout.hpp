#pragma once


#include <planet/ui/element.hpp>

#include <vector>


namespace planet::ui {


    /// ## A layout
    template<typename C>
    class layout {
        C elements;

      public:
        using element_type = typename C::value_type;
        using constrained_type = typename element_type::constrained_type;
        using value_type = typename element_type::value_type;

        layout() = default;

        void clear() { elements.clear(); }
        void push_back(constrained_type c) { elements.emplace_back(c); }
        auto begin() noexcept { return elements.begin(); }
        auto end() noexcept { return elements.end(); }
        auto &back() noexcept { return elements.back(); }

        std::optional<constrained_type> laid_out_in;
        std::optional<affine::extents2d> size;
    };


}
