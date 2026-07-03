#pragma once


namespace planet::telemetry {


    class counter;
    class exponential_decay;
    class id;
    class performance;
    class real_time_decay;
    class real_time_rate;
    class thread_load;
    template<typename Key, typename Value, typename Compare = std::less<Key>>
    class table;


}
