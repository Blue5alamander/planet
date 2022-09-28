#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>

#include <string>
#include <string_view>


namespace planet::io {


    felspar::coro::stream<std::string> connection(
            felspar::coro::stream<felspar::coro::generator<std::string_view>>);


}
