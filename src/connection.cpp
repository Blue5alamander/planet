#include <planet/connection.hpp>


felspar::coro::stream<planet::client::message> planet::client::connection(
        felspar::coro::stream<felspar::coro::generator<std::string_view>>
                commands) {
    while (auto command = co_await commands.next()) {
        std::string expr;
        auto const first = command->next();
        if (not first || first->empty()) {
            // blank line
        } else if (first == "print") {
            for (auto part : *command) {
                if (not expr.empty()) { expr += ' '; }
                expr += part;
            }
            co_yield message{std::move(expr)};
        } else {
            co_yield message{error{"Unknown command", std::string{*first}}};
        }
    }
}
