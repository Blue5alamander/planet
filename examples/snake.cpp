#include <planet/connection.hpp>
#include <planet/hexmap.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <iostream>
#include <random>


namespace {


    struct snake;


    /// Feature within the hex
    enum class feature { none, rock, food, player };
    /// Data structure describing a single hex
    struct hex {
        feature features = {};
        snake *player = nullptr;

        using world_type = planet::hexmap::world_type<hex, 32>;
    };
    std::string to_string(hex const &);


    /// Player state
    enum class player { alive, dead };

    /// Message from game engine to UI
    struct message {
        player state = player::alive;
        /// The view distance from the head of the snake
        long view_distance = 2;
        /// Difference in length between this turn and the last
        long length_delta = 0;
        message &operator+=(long d) {
            length_delta += d;
            return *this;
        }
        /// Error message
        std::string error = {};

        message() {}
        message(player s) : state{s} {}
        message(std::string e) : error{std::move(e)} {}
    };
    std::ostream &operator<<(std::ostream &os, message const &m) {
        if (not m.error.empty()) { return os << m.error << '\n'; }
        if (m.length_delta < 0) {
            if (m.state == player::dead) {
                return os << "Uh oh, you got shorter and died :-(\n";
            } else {
                os << "Uh oh, you got shorter. ";
            }
        }
        if (m.state == player::dead) {
            return os << "Oh no, you died :-(\n";
        } else if (m.length_delta > 0) {
            os << "You grew in length!";
        } else {
            os << "Nothing special happened";
        }
        return os << '\n';
    }


    struct snake {
        std::string name = "you";
        planet::hexmap::coordinates position = {};
        std::vector<hex *> occupies;

        /// Initialise the snake position on the world
        explicit snake(hex::world_type &world) {
            occupies.push_back(&world[position]);
            occupies.back()->player = this;
        }

        /// The snake has moved
        felspar::coro::stream<message>
                move(hex::world_type &world, planet::hexmap::coordinates by) {
            position = position + by;
            auto &h = world[position];
            if (h.player) {
                co_yield {player::dead};
            } else if (h.features == feature::food) {
                co_yield message{} += 1;
                h.features = {};
            } else if (h.features == feature::rock) {
                co_yield {player::dead};
            } else {
                occupies.front()->player = nullptr;
                occupies.erase(occupies.begin());
                co_yield {};
            }
            occupies.push_back(&world[position]);
            occupies.back()->player = this;
        }
    };


    void draw(hex::world_type const &world, snake const &player, long range) {
        auto const top_left =
                player.position + planet::hexmap::coordinates{-range, range};
        auto const bottom_right = player.position
                + planet::hexmap::coordinates{range + 1, -range - 1};
        bool spaces = true;
        std::optional<long> current_row;
        std::string line;
        for (auto const loc :
             planet::hexmap::coordinates::by_column(top_left, bottom_right)) {
            if (loc.row() != current_row) {
                if (line.find_first_not_of(' ') != std::string::npos) {
                    std::cout << line << "\n\n";
                }
                line.clear();
                current_row = loc.row();
                if (spaces = not spaces; spaces) { line += "  "; }
            }
            auto const &cell = world[loc];
            if ((player.position - loc).mag2() > range * range) {
                line += ' ';
            } else if (player.position == loc) {
                line += 'h';
            } else if (cell.player) {
                line += 's';
            } else if (cell.features == feature::rock) {
                line += 'o';
            } else if (cell.features == feature::food) {
                line += '+';
            } else {
                line += '.';
            }
            line += "   ";
        }
        if (line.find_first_not_of(' ') != std::string::npos) {
            std::cout << line << '\n';
        }
    }


    felspar::coro::task<int>
            print(hex::world_type &world,
                  snake &player,
                  felspar::coro::stream<message> messages) {
        while (auto message = co_await messages.next()) {
            std::cout << *message << '\n';
            if (message->state == player::dead) { co_return 1; }
            draw(world, player, message->view_distance);
        }
        co_return 0;
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
        snake player{world};

        std::cout << "Welcome to snake\n\n";
        std::cout << "Type one of ne, nw, w, e, se, sw followed by enter to "
                     "move in that direction\n\n";
        draw(world, player, 4);

        planet::client::command_mapping<message> commands;
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
#ifndef NDEBUG
        commands["draw"] =
                [&world, &player](auto args) -> felspar::coro::stream<message> {
            draw(world, player, 8);
            co_return;
        };
#endif

        co_return co_await print(
                world, player,
                planet::client::connection(planet::io::commands(), commands));
    }


}


int main() { return co_main().get(); }
