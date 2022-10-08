#include <planet/connection.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>


namespace {

    auto command_map = []() {
        planet::client::command_mapping<std::string> c;
        c["print"] = [](auto args) -> felspar::coro::stream<std::string> {
            std::string expr;
            for (auto part : args) {
                if (not expr.empty()) { expr += ' '; }
                expr += part;
            }
            co_yield std::move(expr);
        };
        return c;
    }();

    felspar::coro::task<int> co_main() {
        auto responses =
                planet::client::connection(planet::io::commands(), command_map);
        while (auto message = co_await responses.next()) {
            std::cout << *message << '\n';
        }
        co_return 0;
    }

}


int main() { return co_main().get(); }
