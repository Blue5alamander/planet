#pragma once


#include <felspar/coro/generator.hpp>

#include <filesystem>


namespace planet {


    /// Find paths for asset files
    class asset_manager {
        std::filesystem::path exe_directory;

      public:
        /// Create an asset manager
        asset_manager(std::filesystem::path exe);

        /// The paths that are to be searched for assets
        felspar::coro::generator<std::filesystem::path> search_paths();
    };


}
