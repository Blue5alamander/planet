#pragma once


#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>


namespace planet::drawing {


    /// ## BMP file format
    /**
     * Hard coded to 32 bits per pixel, single plane. Pixel order is BRGA (with
     * increasing memory order).
     */
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

        std::size_t header_size() const noexcept;
        std::size_t pixels_size() const;


      public:
        bmp(std::size_t width, std::size_t height);


        std::vector<std::vector<std::uint8_t>> pixels;


        /// ### Sizes
        std::size_t width() const;
        std::size_t height() const;


        /// ### Save the file
        void save(std::filesystem::path const &) const;


        /// ### Colour accessors
        std::uint8_t &red(std::size_t const x, std::size_t const y) {
            return pixels.at(pixels.size() - y - 1).at(x * 4 + 2);
        }
        std::uint8_t &green(std::size_t const x, std::size_t const y) {
            return pixels.at(pixels.size() - y - 1).at(x * 4 + 1);
        }
        std::uint8_t &blue(std::size_t const x, std::size_t const y) {
            return pixels.at(pixels.size() - y - 1).at(x * 4 + 0);
        }
        std::uint8_t &alpha(std::size_t const x, std::size_t const y) {
            return pixels.at(pixels.size() - y - 1).at(x * 4 + 3);
        }
    };


}
