#include <planet/connection.hpp>

#include <iostream>


felspar::coro::stream<std::string> planet::io::connection(
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
            co_yield std::move(expr);
        } else {
            std::cout << "Unknown command '" << *first << "' -- ignoring\n";
        }
    }
}
