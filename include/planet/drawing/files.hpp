#pragma once


#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>


namespace planet::drawing {


    /// BMP file format
    class bmp {
        struct _file_header {
            std::array<std::uint8_t, 14> storage = {};
            std::size_t size() const noexcept { return storage.size(); }
            _file_header();

            void file_size(std::size_t bytes);
            void image_start(std::size_t pos);
        } file_header;

        struct _bitmap_info_header {
            std::array<std::uint8_t, 40> storage = {};
            std::size_t size() const noexcept { return storage.size(); }
            _bitmap_info_header();

            void extents(std::size_t w, std::size_t h);
        } bitmap_info_header;

        std::vector<std::vector<std::uint8_t>> pixels;

      public:
        bmp(std::size_t width, std::size_t height);

        /// Sizes
        std::size_t header_size() const noexcept;
        std::size_t pixels_size() const;
        std::size_t width() const;
        std::size_t height() const;

        /// Save the file
        void save(std::filesystem::path const &) const;
    };


}
