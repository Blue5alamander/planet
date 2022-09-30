#include <planet/message.hpp>

#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>

#include <functional>
#include <map>
#include <string>
#include <string_view>


namespace planet::client {


    using command_function =
            std::function<felspar::coro::stream<planet::client::message>(
                    felspar::coro::generator<std::string_view>)>;

    using command_mapping = std::map<std::string_view, command_function>;

    felspar::coro::stream<message> connection(
            felspar::coro::stream<felspar::coro::generator<std::string_view>>,
            command_mapping);


}
