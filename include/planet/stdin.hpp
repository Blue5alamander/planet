#pragma once


#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>

#include <string_view>


namespace planet::io {


    felspar::coro::stream<felspar::coro::generator<std::string_view>> commands();


}
