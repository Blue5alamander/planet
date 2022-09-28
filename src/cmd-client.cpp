#include <planet/connection.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>


namespace {
    felspar::coro::task<int> co_main() {
        auto commands = planet::io::connection(planet::io::commands());
        while (auto command = co_await commands.next()) {
            std::cout << *command << '\n';
        }
        co_return 0;
    }
}


int main() {
    std::cout << "Planet game\n";
    return co_main().get();
}
