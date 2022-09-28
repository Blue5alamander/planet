#include <planet/connection.hpp>


felspar::coro::stream<std::string> planet::io::connection(
        felspar::coro::stream<felspar::coro::generator<std::string_view>>
                commands) {
    while (auto command = co_await commands.next()) {
        std::string expr;
        for (auto part : *command) {
            if (not expr.empty()) { expr += ' '; }
            expr += part;
        }
        co_yield std::move(expr);
    }
}
