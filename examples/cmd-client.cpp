#include <planet/connection.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>


namespace {

    auto command_map = []() {
        planet::client::command_mapping c;
        c["print"] =
                [](auto args) -> felspar::coro::stream<planet::client::message> {
            std::string expr;
            for (auto part : args) {
                if (not expr.empty()) { expr += ' '; }
                expr += part;
            }
            co_yield planet::client::message{std::move(expr)};
        };
        return c;
    }();

    felspar::coro::task<int> co_main() {
        auto responses =
                planet::client::connection(planet::io::commands(), command_map);
        while (auto message = co_await responses.next()) {
            if (auto const m =
                        std::get_if<planet::client::error>(&message->payload);
                m) {
                std::cerr << "Error: '" << m->message << "' for '" << m->context
                          << "'\n";
            } else {
                std::cout << std::get<std::string>(message->payload) << '\n';
            }
        }
        co_return 0;
    }

}


int main() { return co_main().get(); }
