#include <planet/connection.hpp>


felspar::coro::stream<planet::client::message> planet::client::connection(
        felspar::coro::stream<felspar::coro::generator<std::string_view>>
                commands,
        command_mapping const &builtins) {
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
            co_yield message{error{"Unknown command", std::string{*first}}};
        }
    }
}
