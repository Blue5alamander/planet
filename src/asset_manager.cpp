#include <planet/asset_manager.hpp>

#include <felspar/exceptions.hpp>

#include <sstream>


planet::asset_manager::asset_manager(std::filesystem::path p)
: exe_directory{p.parent_path()} {}


felspar::coro::generator<std::filesystem::path>
        planet::asset_manager::search_paths() const {
    auto const cwd = std::filesystem::current_path();
    auto const current = (cwd / "var/").lexically_normal();
    co_yield current;
    auto const exe_based = (cwd / exe_directory / "var/").lexically_normal();
    if (exe_based != current) { co_yield exe_based; }
}


std::filesystem::path planet::asset_manager::find_path(
        std::filesystem::path const &fn,
        felspar::source_location const &loc) const {
    std::stringstream ss;
    ss << "Could not find filename " << fn << '\n';
    for (auto path : search_paths()) {
        auto pn = path / fn;
        ss << "Looked for " << pn << '\n';
        if (std::filesystem::exists(pn)) { return {std::move(pn)}; }
    }
    throw felspar::stdexcept::runtime_error{ss.str(), loc};
}
