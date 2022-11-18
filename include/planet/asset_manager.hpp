#pragma once


#include <felspar/coro/generator.hpp>
#include <felspar/test/source.hpp>

#include <filesystem>
#include <vector>


namespace planet {


    /// Find paths for asset files
    class asset_manager {
        std::filesystem::path exe_directory;

      public:
        /// Create an asset manager
        asset_manager(std::filesystem::path exe);

        /// The paths that are to be searched for assets
        felspar::coro::generator<std::filesystem::path> search_paths() const;

        /// Return the full path name for the asset, if it can be found
        std::filesystem::path find_path(
                std::filesystem::path const &,
                felspar::source_location const & =
                        felspar::source_location::current()) const;

        /// Return the file contents
        std::vector<std::byte> file_data(
                std::filesystem::path const &,
                felspar::source_location const & =
                        felspar::source_location::current()) const;
    };


}
