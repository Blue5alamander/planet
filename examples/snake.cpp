#include <planet/connection.hpp>
#include <planet/hexmap.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>


namespace {


    struct snake;


    enum class feature { rock = 1, food = 2 };


    struct hex {
        feature features = {};
        snake *player = nullptr;

        using world_type = planet::hexmap::world_type<hex, 32>;
    };
    std::string to_string(hex const &);


    struct snake {
        std::string name = "you";
        planet::hexmap::coordinate position = {};
        std::vector<hex *> occupies;

        felspar::coro::stream<planet::client::message>
                move(hex::world_type &world, planet::hexmap::coordinate by) {
            if (occupies.empty()) {
                position = by;
                occupies.push_back(&world[position]);
                occupies.back()->player = this;
            } else {
                position = position + by;
                occupies.front()->player = nullptr;
                occupies.erase(occupies.begin());
                occupies.push_back(&world[position]);
                occupies.back()->player = this;
            }
#ifndef NDEBUG
            co_yield planet::client::message{"Location " + to_string(position)};
#endif
            co_yield planet::client::message{
                    "East "
                    + to_string(world[position + planet::hexmap::east])};
            co_yield planet::client::message{
                    "North east "
                    + to_string(world[position + planet::hexmap::north_east])};
            co_yield planet::client::message{
                    "North west "
                    + to_string(world[position + planet::hexmap::north_west])};
            co_yield planet::client::message{
                    "West "
                    + to_string(world[position + planet::hexmap::west])};
            co_yield planet::client::message{
                    "South west "
                    + to_string(world[position + planet::hexmap::south_west])};
            co_yield planet::client::message{
                    "South east "
                    + to_string(world[position + planet::hexmap::south_east])};
        }
    };


    inline std::string to_string(hex const &h) {
        if (h.player) {
            return h.player->name;
        } else {
            return "is empty";
        }
    }


    felspar::coro::task<int> co_main() {
        hex::world_type world{{}, [](auto p) { return hex{}; }};
        snake player;

        std::cout << "Welcome to snake. Your current situation is:\n";
        std::cout << "Type one of ne, nw, w, e, se, sw followed by enter to "
                     "move in that direction\n\n";

        for (auto start = player.move(world, {});
             auto message = co_await start.next();) {
            std::cout << std::get<std::string>(message->payload) << '\n';
        }

        planet::client::command_mapping commands;
        commands["e"] = [&world, &player](auto args) {
            return player.move(world, planet::hexmap::east);
        };
        commands["ne"] = [&world, &player](auto args) {
            return player.move(world, planet::hexmap::north_east);
        };
        commands["nw"] = [&world, &player](auto args) {
            return player.move(world, planet::hexmap::north_west);
        };
        commands["w"] = [&world, &player](auto args) {
            return player.move(world, planet::hexmap::west);
        };
        commands["sw"] = [&world, &player](auto args) {
            return player.move(world, planet::hexmap::south_west);
        };
        commands["se"] = [&world, &player](auto args) {
            return player.move(world, planet::hexmap::south_east);
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
