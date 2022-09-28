#include <felspar/coro/task.hpp>
#include <planet/stdin.hpp>
#include <iostream>


namespace {
    felspar::coro::task<int> co_main() {
        auto commands = planet::io::commands();
        while (auto command = co_await commands.next()) {
            for (auto part : *command) { std::cout << part << ' '; }
            std::cout << '\n';
        }
        co_return 0;
    }
}


int main() {
    std::cout << "Planet game\n";
    return co_main().get();
}
