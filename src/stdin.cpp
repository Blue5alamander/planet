#include <felspar/coro/task.hpp>

#include <planet/stdin.hpp>

#include <iostream>
#include <string>


namespace {
    felspar::coro::generator<std::string_view> lex(std::string line) {
        std::size_t start = line.find_first_not_of(" "),
                    end = line.find_first_of(" ", start);
        while (true) {
            if (end == std::string::npos) {
                co_yield std::string_view{line.data() + start};
                co_return;
            } else {
                co_yield std::string_view{line.data() + start, end - start};
            }
            start = line.find_first_not_of(" ", end);
            end = line.find_first_of(" ", start);
        }
    }
    felspar::coro::task<felspar::coro::generator<std::string_view>> readline() {
        std::cout << "> " << std::flush;
        std::string line;
        std::getline(std::cin, line);
        co_return lex(std::move(line));
    }
}


felspar::coro::stream<felspar::coro::generator<std::string_view>>
        planet::io::commands() {
    while (std::cin) { co_yield co_await readline(); }
}
