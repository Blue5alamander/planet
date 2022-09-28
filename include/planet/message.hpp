#include <string>
#include <variant>


namespace planet::client {


    struct error {
        std::string message, context;
    };


    struct message {
        std::variant<std::string, error> payload;
    };


}
