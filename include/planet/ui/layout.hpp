#pragma once


#include <planet/ui/element.hpp>

#include <felspar/exceptions/runtime_error.hpp>
#include <felspar/memory/small_vector.hpp>

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


        template<typename V>
        void emplace_back(V &&c) {
            elements.emplace_back(std::forward<V>(c));
        }

        std::size_t size() const noexcept { return elements.size(); }


        template<typename I>
        void resize_to(std::span<I, std::dynamic_extent> const s) {
            elements.resize(s.size());
        }
        template<typename I, std::size_t N>
        void resize_to(std::span<I, N>) {}
        template<typename I, std::size_t N>
        void resize_to(std::array<I, N> const &) {}
        template<typename I>
        void resize_to(std::vector<I> const &v) {
            elements.resize(v.size());
        }


        auto begin() noexcept { return elements.begin(); }
        auto begin() const noexcept { return elements.begin(); }
        auto end() noexcept { return elements.end(); }
        auto end() const noexcept { return elements.end(); }
        auto &back() noexcept { return elements.back(); }


        /// ### Safe access to a the layout of a particular element
        element_type &
                at(std::size_t index,
                   std::source_location const &loc =
                           std::source_location::current())
            requires(not requires { elements.at(index, loc); })
        {
            if (index >= size()) {
                throw felspar::stdexcept::runtime_error{
                        "index out of range", loc};
            }
            return elements.at(index);
        }
        element_type &
                at(std::size_t index,
                   std::source_location const &loc =
                           std::source_location::current())
            requires requires { elements.at(index, loc); }
        {
            if (index >= size()) {
                throw felspar::stdexcept::runtime_error{
                        "index out of range", loc};
            }
            return elements.at(index, loc);
        }

        void write_constraints(std::size_t const i, constrained_type c) {
            at(i).constraints = c;
        }

        std::optional<constrained_type> laid_out_in;
        std::optional<affine::extents2d> extents;
    };


    namespace detail {
        template<typename C, typename E>
        struct layouts;

        template<typename E, typename V, std::size_t N>
        struct layouts<std::array<V, N>, E> {
            using layout_type = layout<std::array<element<E>, N>>;
        };
        template<typename E, typename V>
        struct layouts<std::vector<V>, E> {
            using layout_type = layout<std::vector<element<E>>>;
        };
        template<typename E, typename V, std::size_t N>
        struct layouts<felspar::memory::small_vector<V, N>, E> {
            using layout_type =
                    layout<felspar::memory::small_vector<element<E>, N>>;
        };
    }

    template<typename C, typename E = void>
    using layout_for = typename detail::layouts<C, E>::layout_type;


}
