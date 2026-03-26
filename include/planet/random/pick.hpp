#pragma once


#include <planet/random/generator.hpp>

#include <felspar/exceptions/runtime_error.hpp>

#include <span>


namespace planet::random {


    /// ## Random selection
    template<typename G, std::integral N>
    inline N pick_number(G &gen, N const lowest, N const highest) {
        return std::uniform_int_distribution<N>{lowest, highest}(gen);
    }
    template<std::integral N>
    inline N pick_number(N const lowest, N const highest) {
        return pick_number(generator, lowest, highest);
    }


    /// ## Choose one

    /// ### From a potentially empty collection
    template<typename G, typename T>
    inline T const *maybe_pick_one(G &gen, std::span<T const> const s) {
        if (s.empty()) {
            return nullptr;
        } else {
            return &s[pick_number(gen, 0uz, s.size() - 1uz)];
        }
    }
    template<typename T>
    inline T const *maybe_pick_one(std::span<T const> const s) {
        return maybe_pick_one(generator, s);
    }
    template<typename T>
    inline T const *maybe_pick_one(std::vector<T> const &s) {
        return maybe_pick_one(generator, std::span{s.data(), s.size()});
    }

    /// ### From a collection that must have members
    template<typename G, typename T>
    inline T const &pick_one(
            G &gen,
            std::span<T const> const s,
            std::source_location const &loc = std::source_location::current()) {
        if (s.empty()) {
            throw felspar::stdexcept::runtime_error{
                    "There are no items to pick one of", loc};
        } else {
            return s[std::uniform_int_distribution<std::size_t>{
                    0, s.size() - 1}(gen)];
        }
    }
    template<typename T>
    inline T const &pick_one(
            std::span<T const> const s,
            std::source_location const &loc = std::source_location::current()) {
        return pick_one(generator, s, loc);
    }
    template<typename T, std::size_t N>
    inline T const &pick_one(
            std::array<T, N> const &s,
            std::source_location const &loc = std::source_location::current()) {
        return pick_one(generator, std::span{s.data(), s.size()}, loc);
    }
    template<typename T>
    inline T const &pick_one(
            std::vector<T> const &s,
            std::source_location const &loc = std::source_location::current()) {
        return pick_one(generator, std::span{s.data(), s.size()}, loc);
    }
}
