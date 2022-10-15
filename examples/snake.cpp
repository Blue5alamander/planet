#include <planet/connection.hpp>
#include <planet/hexmap.hpp>
#include <planet/stdin.hpp>

#include <felspar/coro/task.hpp>

#include <charconv>
#include <iostream>
#include <random>


namespace {


    /// Some configuration
    using prng_type = std::mt19937;
    using distribution_type = std::uniform_real_distribution<float>;


    /// The probability increases with distance
    inline auto increasing(float control) {
        return [control](
                       auto &generator, auto &distribution,
                       float const distance) -> bool {
            auto const probability = distance / (distance + control);
            return distribution(generator) < probability;
        };
    }
    inline auto decreasing(float control) {
        return [control](
                       auto &generator, auto &distribution,
                       float const distance) -> bool {
            auto const probability = distance / (distance + control);
            return distribution(generator) > probability;
        };
    }


    /// Feature within the hex -- keep `player` last when adding new ones
    enum class feature { none, rock, food, food_plus };
    /// Determine how a feature is generated.
    struct generate_feature {
        feature creates;
        std::function<auto(std::mt19937 &,
                           std::uniform_real_distribution<float> &,
                           float)
                              ->bool>
                determine;
    };


    /// Functions for creating each type of feature in precedence order. The
    /// first to return true will determine the feature in that hex tile.
    std::array<generate_feature, 4> const map_options = {
            generate_feature{
                    feature::none,
                    [](auto &, auto &, float const distance) {
                        return distance <= 1.0f;
                    }},
            generate_feature{feature::rock, increasing(16.0f)},
            generate_feature{feature::food, decreasing(0.6f)},
            generate_feature{feature::food_plus, increasing(100.0f)}};


    /// Data structure describing a single hex tile
    struct snake;
    struct hex {
        feature features = feature::none;
        snake *player = nullptr;

        using world_type = planet::hexmap::world_type<hex, 32>;
    };


    /// Player state
    enum class player { alive, dead };


    /// Message from game engine to UI
    struct message {
        player state = player::alive;
        /// The view distance from the head of the snake
        long view_distance = 2;
        /// Difference in length between this turn and the last
        long length_delta = 0;
        /// Change in health
        long health_delta = 0;
        /// Change in score
        long score_delta = {};
        /// Error message
        std::string error = {};

        message() {}
        message(player s) : state{s} {}
        message(std::string e) : error{std::move(e)} {}
    };
    std::ostream &operator<<(std::ostream &os, message const &m) {
        if (not m.error.empty()) {
            return os << m.error << '\n';
        } else if (m.state == player::dead) {
            return os << "Oh no, you died :-(\n";
        } else if (m.length_delta < 0) {
            return os << "Uh oh, you got shorter\n";
        } else if (m.length_delta > 0) {
            return os << "You grew in length!\n";
        } else {
            return os << "Nothing special happened\n";
        }
    }


    struct snake {
        std::string name = "you";
        planet::hexmap::coordinates position = {};
        std::vector<hex *> occupies;
        long health = 5;
        long score = {};

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
            message outcome{};
            if (h.player) {
                outcome.state = player::dead;
            } else {
                occupies.push_back(&world[position]);
                occupies.back()->player = this;
                outcome.length_delta += 1;
                outcome.health_delta -= 2;

                switch (h.features) {
                case feature::none: break;
                case feature::food:
                    outcome.health_delta += 9;
                    outcome.score_delta += 5;
                    break;
                case feature::food_plus:
                    outcome.health_delta += 14;
                    outcome.score_delta += 6;
                    break;
                case feature::rock: outcome.health_delta -= 12; break;
                }
            }
            co_yield outcome;
        }

        felspar::coro::stream<message>
                process_outcome(felspar::coro::stream<message> stream) {
            while (auto outcome = co_await stream.next()) {
                health += outcome->health_delta;
                score += outcome->score_delta;
                if (health < 0) { outcome->state = player::dead; }
                auto const length = health / 8;
                while (occupies.size() > length) {
                    occupies.front()->player = nullptr;
                    occupies.erase(occupies.begin());
                    outcome->length_delta = -1;
                }
                co_yield *outcome;
            }
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
                line += 'f';
            } else if (cell.features == feature::food_plus) {
                line += 'F';
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
            if (message->state == player::dead) {
                std::cout << "Your final score was " << player.score << '\n';
                co_return 1;
            }
            if (message->error.empty()) {
                draw(world, player, message->view_distance);
            }
#ifndef NDEBUG
            std::cout << "Score: " << player.score
                      << " health: " << player.health << '\n';
#endif
        }
        co_return 0;
    }


    felspar::coro::task<int> co_main() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

        hex::world_type world{
                {}, [&gen, distribution](auto const p) mutable {
                    auto const dist = std::sqrt(p.mag2());
                    for (auto const &f : map_options) {
                        if (f.determine(gen, distribution, dist)) {
                            return hex{f.creates};
                        }
                    }
                    return hex{};
                }};
        snake player{world};

        std::cout << "Welcome to snake\n\n";
        std::cout << "Type one of 'ne', 'nw', 'w', 'e', 'se', 'sw' followed by "
                     "enter to move in that direction\n";
        std::cout << "You can also ask to 'draw' the map\n\n";
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
        commands["draw"] =
                [&world, &player](auto args) -> felspar::coro::stream<message> {
            std::string_view const sv = args.next().value_or("8");
            message distance{};
            auto const [_, ec] = std::from_chars(
                    sv.data(), sv.data() + sv.size(), distance.view_distance);
            if (ec != std::errc{}) {
                co_yield message{
                        "Sorry, I could not understand the distance, using 8"};
                distance.view_distance = 8;
            }
            distance.health_delta -= distance.view_distance;
            co_yield distance;
        };

        co_return co_await print(
                world, player,
                player.process_outcome(planet::client::connection(
                        planet::io::commands(), commands)));
    }


}


int main() { return co_main().get(); }