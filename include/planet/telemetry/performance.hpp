#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/telemetry/id.hpp>


namespace planet::telemetry {


    /// ## One of any number of performance counters
    class performance : public id {
      public:
        performance(std::string_view);
        virtual ~performance();


        /// ### Save all non-zero performance counters
        /**
         * Values are saved into the buffer without a length prefix. The
         * returned value is the number of items that were written.
         */
        static std::size_t current_values(serialise::save_buffer &);

      private:
        virtual bool save(serialise::save_buffer &) = 0;
    };


}
