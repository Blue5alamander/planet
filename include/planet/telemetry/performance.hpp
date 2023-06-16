#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/telemetry/id.hpp>


namespace planet::telemetry {


    /// ## One of any number of performance counters
    class performance : public id {
      public:
        performance(std::string_view);
        virtual ~performance();


      private:
        virtual void save(serialise::save_buffer &) = 0;
    };


}
