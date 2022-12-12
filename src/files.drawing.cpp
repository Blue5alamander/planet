#include <planet/drawing/files.hpp>

#include <fstream>


namespace {


    template<std::size_t N>
    void write_le(
            std::array<std::uint8_t, N> &a,
            std::size_t const i,
            std::uint32_t const v) {
        a[i + 0] = v;
        a[i + 1] = (v >> 8);
        a[i + 2] = (v >> 16);
        a[i + 3] = (v >> 24);
    }


}


planet::drawing::bmp::_file_header::_file_header() {
    storage[0] = 'B';
    storage[1] = 'M';
}


void planet::drawing::bmp::_file_header::file_size(std::size_t const bytes) {
    write_le(storage, 2, bytes);
}


void planet::drawing::bmp::_file_header::image_start(std::size_t const pos) {
    write_le(storage, 10, pos);
}


planet::drawing::bmp::_bitmap_info_header::_bitmap_info_header() {
    write_le(storage, 0, size());
    storage[12] = 1; // planes
    storage[14] = 32; // bits per pixel
    write_le(storage, 24, 2835); // 72dpi
    write_le(storage, 28, 2835);
}


void planet::drawing::bmp::_bitmap_info_header::extents(
        std::size_t const w, std::size_t const h) {
    write_le(storage, 4, w);
    write_le(storage, 8, h);
}


planet::drawing::bmp::bmp(std::size_t const width, std::size_t const height)
: pixels(height, std::vector<std::uint8_t>(width * 4)) {
    file_header.file_size(header_size() + pixels_size());
    file_header.image_start(header_size());
    bitmap_info_header.extents(width, height);
}


std::size_t planet::drawing::bmp::header_size() const noexcept {
    return file_header.size() + bitmap_info_header.size();
}
std::size_t planet::drawing::bmp::pixels_size() const {
    return pixels.size() * pixels.at(0).size();
}
std::size_t planet::drawing::bmp::width() const {
    return pixels.at(0).size() / 4;
}
std::size_t planet::drawing::bmp::height() const { return pixels.size(); }


void planet::drawing::bmp::save(std::filesystem::path const &fn) const {
    std::ofstream file{fn, std::ios_base::out | std::ios_base::binary};
    file.write(
            reinterpret_cast<char const *>(file_header.storage.data()),
            file_header.size());
    file.write(
            reinterpret_cast<char const *>(bitmap_info_header.storage.data()),
            bitmap_info_header.size());
    for (auto &row : pixels) {
        file.write(reinterpret_cast<char const *>(row.data()), row.size());
    }
}
