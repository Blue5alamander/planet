#include <planet/asset_manager.hpp>


planet::asset_manager::asset_manager(std::filesystem::path p)
: exe_directory{p.parent_path()} {}


felspar::coro::generator<std::filesystem::path>
        planet::asset_manager::search_paths() {
    auto const cwd = std::filesystem::current_path();
    auto const current = (cwd / "var/").lexically_normal();
    co_yield current;
    auto const exe_based = (cwd / exe_directory / "var/").lexically_normal();
    if (exe_based != current) { co_yield exe_based; }
}
