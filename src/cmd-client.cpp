#include <planet/connection.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>


namespace {
    felspar::coro::task<int> co_main() {
        auto responses = planet::client::connection(planet::io::commands());
        while (auto message = co_await responses.next()) {
            std::cout << std::get<std::string>(message->payload) << '\n';
        }
        co_return 0;
    }
}


int main() {
    std::cout << "Planet game\n";
    return co_main().get();
}
