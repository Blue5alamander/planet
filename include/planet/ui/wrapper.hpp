#pragma once


#include <planet/ui/reflowable.hpp>

#include <felspar/coro/eager.hpp>

#include <concepts>
#include <memory>


namespace planet::ui {


    namespace detail {
        /// ## Type erased wrapper
        template<typename... Args>
        struct abstract_wrapper {
            virtual ~abstract_wrapper() = default;


            using constrained_type = planet::ui::reflowable::constrained_type;
            using reflow_parameters = planet::ui::reflowable::reflow_parameters;


            virtual void do_update(Args...) {};
            virtual constrained_type
                    reflow(reflow_parameters const &,
                           constrained_type const &) = 0;
            virtual planet::affine::rectangle2d
                    move_to(reflow_parameters const &,
                            planet::affine::rectangle2d const &) = 0;
            virtual void do_draw() = 0;
        };
    }


    /// ## Wrapper for handling layout
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


    namespace detail {
        /// ## Wrapper type that holds the graphic
        template<typename T, typename... Args>
        struct graphic_wrapper : public abstract_wrapper<Args...> {
            using superclass = abstract_wrapper<Args...>;
            using constrained_type = typename superclass::constrained_type;
            using reflow_parameters = typename superclass::reflow_parameters;
            using content_type = T;


            graphic_wrapper(content_type t) : graphic{std::move(t)} {}


          protected:
            T graphic;


            constrained_type
                    reflow(reflow_parameters const &p,
                           constrained_type const &c) override {
                return graphic.reflow(p, c);
            }
            planet::affine::rectangle2d
                    move_to(reflow_parameters const &p,
                            planet::affine::rectangle2d const &r) override {
                return graphic.move_to(p, r);
            }
            void do_draw() override { graphic.draw(); }
        };


        /// ## The actual wrapper styles that are stored

        template<typename T>
        struct concrete_no_update_wrapper final : public graphic_wrapper<T> {
            using superclass = graphic_wrapper<T>;


            concrete_no_update_wrapper(T t) : superclass{std::move(t)} {}
        };

        template<typename T, typename U, typename... Args>
        struct concrete_lambda_wrapper final :
        public graphic_wrapper<T, Args...> {
            using superclass = graphic_wrapper<T, Args...>;


            concrete_lambda_wrapper(T t, U u)
            : superclass{std::move(t)}, updater{std::move(u)} {}


          private:
            U updater;


            void do_update(Args... args) override {
                updater(superclass::graphic, std::forward<Args>(args)...);
            }
        };

        template<typename T>
        struct concrete_coro_wrapper final : public graphic_wrapper<T> {
            using superclass = graphic_wrapper<T>;


            template<typename U>
            concrete_coro_wrapper(T t, U c)
            : superclass{std::move(t)}, coro{c(superclass::graphic)} {}


          private:
            felspar::coro::eager<> coro;
        };
    }


    /// ## Helpers to create a wrapper instance
    /**
     * When using an update lambda, the compiler can't generally deduce the
     * parameter pack from the lambda argument, so you must explicitly pass the
     * pack parameters to `create_wrapper`.
     */
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


}
