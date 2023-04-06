#pragma once


#include <planet/ui/element.hpp>

#include <vector>


namespace planet::ui {


    /// ## A layout
    template<typename C>
    class layout final {
        C elements;

      public:
        using collection_type = C;
        using element_type = typename C::value_type;
        using constrained_type = typename element_type::constrained_type;

        layout() = default;
        layout(collection_type &&e) : elements{std::move(e)} {}
        layout(collection_type const &e) : elements{e} {}

        void clear() { elements.clear(); }
        void push_back(constrained_type c) { elements.emplace_back(c); }

        auto begin() noexcept { return elements.begin(); }
        auto end() noexcept { return elements.end(); }
        auto &back() noexcept { return elements.back(); }

        element_type &operator[](std::size_t index) {
            return elements.at(index);
        }

        std::optional<constrained_type> laid_out_in;
        std::optional<affine::extents2d> size;
    };


}
