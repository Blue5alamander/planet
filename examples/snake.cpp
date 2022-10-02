#include <planet/connection.hpp>
#include <planet/hexmap.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>
#include <random>


namespace {


    struct snake;


    enum class feature { none = 0, rock = 1, food = 2 };


    struct hex {
        feature features = {};
        snake *player = nullptr;

        using world_type = planet::hexmap::world_type<hex, 32>;
    };
    std::string to_string(hex const &);


    struct snake {
        std::string name = "you";
        planet::hexmap::coordinates position = {};
        std::vector<hex *> occupies;

        felspar::coro::stream<planet::client::message>
                move(hex::world_type &world, planet::hexmap::coordinates by) {
            if (occupies.empty()) {
                position = by;
                occupies.push_back(&world[position]);
                occupies.back()->player = this;
            } else {
                position = position + by;
                auto &h = world[position];
                if (h.player) {
                    co_yield {planet::client::error{
                            "You tried to eat yourself, and so died", {}}};
                } else if (h.features == feature::food) {
                    co_yield planet::client::message{
                            "You have eaten some food"};
                    h.features = {};
                } else if (h.features == feature::rock) {
                    co_yield {planet::client::error{
                            "You hit a rock and died", {}}};
                } else {
                    occupies.front()->player = nullptr;
                    occupies.erase(occupies.begin());
                }
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


    std::string to_string(hex const &h) {
        if (h.player) {
            return h.player->name;
        } else if (h.features == feature::rock) {
            return "has a rock";
        } else if (h.features == feature::food) {
            return "has food";
        } else {
            return "is empty";
        }
    }


    felspar::coro::task<int> co_main() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> probability(0.0f, 1.0f);

        hex::world_type world{{}, [&gen, probability](auto const p) mutable {
                                  auto const dist = std::sqrt(p.mag2());
                                  auto const p_rock = dist / (dist + 15.8f);
                                  auto const p_food = dist / (dist + 0.6f);
                                  if (not dist) {
                                      return hex{};
                                  } else if (probability(gen) < p_rock) {
                                      return hex{feature::rock};
                                  } else if (probability(gen) > p_food) {
                                      return hex{feature::food};
                                  } else {
                                      return hex{};
                                  }
                              }};
        snake player;

        std::cout << "Welcome to snake\n\n";
        std::cout << "Type one of ne, nw, w, e, se, sw followed by enter to "
                     "move in that direction\n\n";
        std::cout << "Your current situation is:\n";

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
