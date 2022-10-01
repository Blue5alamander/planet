#include <planet/connection.hpp>
#include <planet/hexmap.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>


namespace {


    enum class feature { rock = 1, food = 2 };


    struct hex {
        feature features = {};
    };


    struct snake {
        planet::hexmap::coordinate position = {};

        felspar::coro::stream<planet::client::message>
                move(planet::hexmap::coordinate by) {
            position = position + by;
            co_yield planet::client::message{"Location " + to_string(position)};
        }
    };


    felspar::coro::task<int> co_main() {
        snake player;

        std::cout << "Welcome to snake. Your current situation is:\n";
        for (auto start = player.move({});
             auto message = co_await start.next();) {
            std::cout << std::get<std::string>(message->payload) << '\n';
        }

        planet::client::command_mapping commands;
        commands["ne"] = [&player](auto args) {
            return player.move(planet::hexmap::north_east);
        };
        auto responses =
                planet::client::connection(planet::io::commands(), commands);
        while (auto message = co_await responses.next()) {
            std::cout << std::get<std::string>(message->payload) << '\n';
        }
        co_return 0;
    }


}


int main() { return co_main().get(); }
