#pragma once


#include <planet/ui/wrapper.detail.hpp>

#include <concepts>
#include <memory>


namespace planet::ui {


    /// ## Wrapper for handling layout
    /**
     * This type can be used to type erase part of a user interface. Use one of
     * the `create_wrapper` functions below to actually make the `wrapper`
     * instance depending on what you need.
     */
    template<typename... Args>
    class wrapper final : public planet::ui::reflowable {
        using abstract_wrapper_type = detail::abstract_wrapper<Args...>;
        std::unique_ptr<abstract_wrapper_type> implementation;


      public:
        wrapper() : reflowable{"planet::ui::wrapper"} {}
        wrapper(std::unique_ptr<abstract_wrapper_type> i)
        : reflowable{"planet::ui::wrapper"}, implementation{std::move(i)} {}
        wrapper(std::string_view const n,
                std::unique_ptr<abstract_wrapper_type> i)
        : reflowable{n}, implementation{std::move(i)} {}


        void update(Args... args) {
            if (implementation) {
                implementation->do_update(std::forward<Args>(args)...);
            }
        }
        void draw() {
            if (implementation) { implementation->do_draw(); }
        }


      protected:
        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &c) override {
            if (implementation) {
                return implementation->reflow(p, c);
            } else {
                return {};
            }
        }
        planet::affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                planet::affine::rectangle2d const &r) override {
            if (implementation) {
                return implementation->move_to(p, r);
            } else {
                return {};
            }
        }
    };


    /// ## Helpers to create a wrapper instance

    /// ### No updates
    template<typename T>
    inline auto create_wrapper(T t) {
        return wrapper<>{
                std::make_unique<detail::concrete_no_update_wrapper<T>>(
                        std::move(t))};
    }
    template<typename T>
    inline auto create_wrapper(std::string_view const n, T t) {
        return wrapper<>{
                n,
                std::make_unique<detail::concrete_no_update_wrapper<T>>(
                        std::move(t))};
    }

    /// ### With updates
    /**
     * When using an update lambda, the compiler can't generally deduce the
     * parameter pack from the lambda argument, so you must explicitly pass the
     * pack parameters to `create_wrapper`.
     */
    template<typename... Pack, typename T, std::invocable<T &, Pack...> U>
    inline auto create_wrapper(T t, U u)
        requires std::same_as<std::invoke_result_t<U, T &, Pack...>, void>
    {
        return wrapper<Pack...>{
                std::make_unique<detail::concrete_lambda_wrapper<T, U, Pack...>>(
                        std::move(t), std::move(u))};
    }
    template<typename... Pack, typename T, std::invocable<T &, Pack...> U>
    inline auto create_wrapper(std::string_view const n, T t, U u)
        requires std::same_as<std::invoke_result_t<U, T &, Pack...>, void>
    {
        return wrapper<Pack...>{
                n,
                std::make_unique<detail::concrete_lambda_wrapper<T, U, Pack...>>(
                        std::move(t), std::move(u))};
    }

    /// ### With coroutines
    template<typename T, std::invocable<T &> U>
    inline auto create_wrapper(std::string_view const n, T t, U c)
        requires std::same_as<
                std::invoke_result_t<U, T &>,
                felspar::coro::eager<>::task_type>
    {
        return wrapper<>{
                n,
                std::make_unique<detail::concrete_coro_wrapper<T>>(
                        std::move(t), std::move(c))};
    }

    /// ### With update lambda and coroutines
    template<typename... Pack, typename T, typename C, typename U>
        requires std::invocable<C, T &>
            && std::same_as<
                         std::invoke_result_t<C, T &>,
                         felspar::coro::eager<>::task_type>
            && std::invocable<U, T &, Pack...>
            && std::same_as<std::invoke_result_t<U, T &, Pack...>, void>
    inline auto create_wrapper(std::string_view const n, T t, C c, U u) {
        return wrapper<Pack...>{
                n,
                std::make_unique<
                        detail::concrete_coro_and_lambda_wrapper<T, U, Pack...>>(
                        std::move(t), std::move(c), std::move(u))};
    }


}
