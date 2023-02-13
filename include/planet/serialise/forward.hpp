#pragma once


#include <felspar/parse/concepts.hpp>

#include <span>
#include <string_view>


namespace planet::serialise {


    template<felspar::parse::concepts::integral T>
    void load(std::span<std::byte const> &, T &);
    void load(std::span<std::byte const> &, std::string_view &);


    template<typename S>
    S load_type(std::span<std::byte const> &);


    class save_buffer;


}
