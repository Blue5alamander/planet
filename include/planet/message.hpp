#include <string>
#include <variant>


namespace planet::client {


    struct message {
        std::variant<std::string> payload;
    };


}
