#pragma once


#include <felspar/coro/generator.hpp>
#include <felspar/test/source.hpp>

#include <filesystem>
#include <vector>


namespace planet {


    /// Super class for an asset loader. Create a sub-class of this to load assets
    struct asset_loader {
        asset_loader();
        virtual ~asset_loader();

        virtual std::optional<std::vector<std::byte>> try_load(
                std::ostream &log,
                std::filesystem::path const &fn,
                felspar::source_location const &loc) const = 0;
    };


    /// A file loader
    class file_loader final : public planet::asset_loader {
        std::filesystem::path base;

        std::vector<std::byte>
                file_data(std::filesystem::path const &pathname) const;

      public:
        explicit file_loader(std::filesystem::path);

        /// Try to load the specified asset from various file system paths
        std::optional<std::vector<std::byte>> try_load(
                std::ostream &log,
                std::filesystem::path const &fn,
                felspar::source_location const &loc) const override;

        /// The paths that are to be searched for assets
        felspar::coro::generator<std::filesystem::path> search_paths() const;
    };


    /// Find paths for asset files
    class asset_manager final {
        file_loader exe_directory;

      public:
        /// Create an asset manager
        asset_manager(std::filesystem::path exe);

        /// Return the file contents or throw
        std::vector<std::byte> file_data(
                std::filesystem::path const &,
                felspar::source_location const & =
                        felspar::source_location::current()) const;
    };


}
