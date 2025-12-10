#include <planet/log.hpp>
#include <planet/serialise.hpp>
#include <planet/version.hpp>

#include <optional>
#include <sstream>


namespace {
    std::optional<std::uint16_t> number(std::string_view &sv) {
        if (sv.empty()) {
            return {};
        } else if (auto const dot = sv.find('.');
                   dot == std::string_view::npos) {
            auto const v = std::atoi(std::string{sv}.c_str());
            sv = {};
            return v;
        } else {
            auto const v = std::atoi(
                    std::string{sv.begin(), sv.begin() + dot}.c_str());
            sv = sv.substr(dot + 1);
            return v;
        }
    }
    auto parse(std::string_view sv) {
        auto const major = number(sv);
        auto const minor = number(sv);
        auto const patch = number(sv);
        return planet::semver{
                .major = major.value_or(0),
                .minor = minor.value_or(0),
                .patch = patch.value_or(0)};
    }
}


planet::version::version(std::string_view const id, std::string_view const sv)
: version{id, id, sv} {}
planet::version::version(
        std::string_view const id,
        std::string_view const sv,
        std::uint16_t const b)
: version{id, id, sv, b} {}
planet::version::version(
        std::string_view const id,
        std::string_view const dir,
        std::string_view const sv)
: application_id{id},
  application_folder{dir},
  version_string{sv},
  semver{parse(sv)} {}
planet::version::version(
        std::string_view const id,
        std::string_view const dir,
        std::string_view const sv,
        std::uint16_t const b)
: application_id{id},
  application_folder{dir},
  version_string{sv},
  semver{parse(sv)},
  build{b} {}
planet::version::version(
        std::string_view const id,
        std::string_view const dir,
        std::string_view const sv,
        std::uint16_t const b,
        std::string_view const gd)
: application_id{id},
  application_folder{dir},
  version_string{sv},
  semver{parse(sv)},
  build{b},
  git_describe{gd} {}

planet::version::version(serialise::box &b) { load(b, *this); }


std::ostream &planet::operator<<(std::ostream &os, version const &v) {
    os << v.version_string;
    if (v.build) { os << " build " << *v.build; }
    return os;
}


std::string planet::to_string(version const &v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
}


void planet::save(serialise::save_buffer &sb, semver const &sv) {
    sb.save_box(sv.box, sv.major, sv.minor, sv.patch);
}
void planet::load(serialise::box &b, semver &sv) {
    b.named(sv.box, sv.major, sv.minor, sv.patch);
}
void planet::save(serialise::save_buffer &sb, version const &v) {
    sb.save_box(
            v.box, v.application_id, v.version_string, v.semver, v.build,
            v.git_describe);
}
void planet::load(serialise::box &b, version &v) {
    b.lambda(v.box, [&]() {
        b.fields(v.application_id, v.version_string, v.semver, v.build);
        if (not b.content.empty()) { b.fields(v.git_describe); }
    });
}


namespace {
    auto const print_version = planet::log::format(
            planet::version::box,
            [](std::ostream &os, planet::serialise::box &box, std::size_t) {
                planet::version version{box};
                os << version.application_id << ' ' << version.version_string
                   << " (" << version.semver.major << '.'
                   << version.semver.minor << '.' << version.semver.patch
                   << ')';
                if (version.build) {
                    os << " build " << *version.build;
                } else {
                    os << " (no build number)";
                }
                if (not version.git_describe.empty()) {
                    os << ' ' << version.git_describe;
                }
            });
}
