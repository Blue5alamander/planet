#include <felspar/coro/generator.hpp>
#include <felspar/coro/stream.hpp>


namespace planet::io {


    felspar::coro::stream<felspar::coro::generator<std::string_view>> commands();


}
