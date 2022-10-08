#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>

#include <functional>
#include <map>
#include <string>
#include <string_view>


namespace planet::client {


    template<typename Message>
    using command_function = std::function<felspar::coro::stream<Message>(
            felspar::coro::generator<std::string_view>)>;

    template<typename Message>
    using command_mapping =
            std::map<std::string_view, command_function<Message>>;


    template<typename Message>
    inline felspar::coro::stream<Message> connection(
            felspar::coro::stream<felspar::coro::generator<std::string_view>>
                    commands,
            command_mapping<Message> const &builtins) {
        while (auto command = co_await commands.next()) {
            auto const first = command->next();
            if (not first || first->empty()) {
                // blank line
            } else if (auto function = builtins.find(*first);
                       function != builtins.end()) {
                for (auto messages = function->second(std::move(*command));
                     auto msg = co_await messages.next();) {
                    co_yield std::move(*msg);
                }
            } else {
                co_yield "Unknown command: " + std::string{*first};
            }
        }
    }


}
