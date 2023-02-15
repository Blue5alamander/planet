#pragma once


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


}
