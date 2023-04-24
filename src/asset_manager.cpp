#include <planet/asset_manager.hpp>

#include <felspar/exceptions.hpp>

#include <fstream>
#include <mutex>
#include <sstream>


/// ## `planet::asset_loader`


namespace {
    auto &g_loaders_mtx() {
        static std::mutex m;
        return m;
    }
    auto &g_loaders() {
        static std::vector<planet::asset_loader const *> v;
        return v;
    }
}


planet::asset_loader::asset_loader() {
    std::scoped_lock _{g_loaders_mtx()};
    g_loaders().push_back(this);
}


planet::asset_loader::~asset_loader() {
    std::scoped_lock _{g_loaders_mtx()};
    std::erase(g_loaders(), this);
}


/// ## `planet::asset_manager`


planet::asset_manager::asset_manager(std::filesystem::path p)
: exe_directory{p.parent_path()} {}


std::vector<std::byte> planet::asset_manager::file_data(
        std::filesystem::path const &fn,
        felspar::source_location const &loc) const {
    std::stringstream ss;
    ss << "Could not find filename " << fn << '\n';
    std::scoped_lock _{g_loaders_mtx()};
    ss << g_loaders().size() << " asset loader(s) were tried for " << fn
       << '\n';
    for (auto const *loader : g_loaders()) {
        if (auto data = loader->try_load(ss, fn, loc); data) { return *data; }
    }
    throw felspar::stdexcept::runtime_error{ss.str(), loc};
}


/// ## `planet::file_loader`


planet::file_loader::file_loader(std::filesystem::path b)
: base{std::move(b)} {}


felspar::coro::generator<std::filesystem::path>
        planet::file_loader::search_paths() const {
    auto const cwd = std::filesystem::current_path();
    auto const current = (cwd / "share/").lexically_normal();
    co_yield current;
    auto const exe_based = (cwd / base / "share/").lexically_normal();
    if (exe_based != current) { co_yield exe_based; }
    auto const exe_parent =
            ((cwd / base).parent_path() / "share/").lexically_normal();
    if (exe_parent != exe_based and exe_parent != current) {
        co_yield exe_parent;
    }
}


std::optional<std::vector<std::byte>> planet::file_loader::try_load(
        std::ostream &log,
        std::filesystem::path const &fn,
        felspar::source_location const &) const {
    for (auto path : search_paths()) {
        auto pn = path / fn;
        log << "Looked for " << pn << '\n';
        if (std::filesystem::exists(pn)) { return file_data(pn); }
    }
    return {};
}


std::vector<std::byte>
        planet::file_loader::file_data(std::filesystem::path const &pathname) {
    std::vector<std::byte> d(std::filesystem::file_size(pathname));
    std::ifstream file{pathname};
    file.read(reinterpret_cast<char *>(d.data()), d.size());
    return d;
}
