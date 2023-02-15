#pragma once


#include <planet/serialise/marker.hpp>

#include <felspar/exceptions.hpp>


namespace planet::serialise {


    /// ## Serialisation exceptions


    /// ### Super-class for all serialisation errors
    class serialisation_error : public felspar::stdexcept::runtime_error {
      public:
        serialisation_error(std::string, felspar::source_location const &);
    };


    /// ### Box name too long
    class box_name_length : public serialisation_error {
      public:
        box_name_length(
                std::string_view name,
                felspar::source_location const & =
                        felspar::source_location::current());
    };


    /// ### A box still has data after reading
    class box_not_empty : public serialisation_error {
      public:
        box_not_empty(
                felspar::source_location const & =
                        felspar::source_location::current());
    };


    /// ### Marker errors
    class wanted_boolean : public serialisation_error {
      public:
        wanted_boolean(
                marker got,
                felspar::source_location const & =
                        felspar::source_location::current());
    };
    class wrong_marker : public serialisation_error {
      public:
        wrong_marker(
                marker exected,
                marker got,
                felspar::source_location const & =
                        felspar::source_location::current());
    };


}
