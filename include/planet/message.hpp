#include <string>
#include <variant>


namespace planet::client {


    struct error {
        std::string message, context;
    };


    template<class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    struct message {
        std::variant<std::string, error> payload;

        template<typename... Funcs>
        auto operator()(Funcs &&...fs) -> decltype(auto) {
            return std::visit(overloaded{std::forward<Funcs>(fs)...}, payload);
        }
    };


}
