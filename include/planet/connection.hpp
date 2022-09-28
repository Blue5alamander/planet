#include <planet/message.hpp>

#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>

#include <string>
#include <string_view>


namespace planet::client {


    felspar::coro::stream<message> connection(
            felspar::coro::stream<felspar::coro::generator<std::string_view>>);


}
