#pragma once


#include <planet/ui/reflowable.hpp>

#include <felspar/coro/eager.hpp>


namespace planet::ui::detail {


    /// ## Type erased wrapper
    template<typename... Args>
    struct abstract_wrapper {
        virtual ~abstract_wrapper() = default;


        using constrained_type = planet::ui::reflowable::constrained_type;
        using reflow_parameters = planet::ui::reflowable::reflow_parameters;


        virtual void do_update(Args...) {};
        virtual constrained_type
                reflow(reflow_parameters const &, constrained_type const &) = 0;
        virtual planet::affine::rectangle2d
                move_to(reflow_parameters const &,
                        planet::affine::rectangle2d const &) = 0;
        virtual void do_draw() = 0;
    };


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
    struct concrete_lambda_wrapper final : public graphic_wrapper<T, Args...> {
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

    template<typename T, typename U, typename... Args>
    struct concrete_coro_and_lambda_wrapper final :
    public graphic_wrapper<T, Args...> {
        using superclass = graphic_wrapper<T, Args...>;


        template<typename Coro>
        concrete_coro_and_lambda_wrapper(T t, Coro c, U u)
        : superclass{std::move(t)},
          coro{c(superclass::graphic)},
          updater{std::move(u)} {}


      private:
        felspar::coro::eager<> coro;
        U updater;


        void do_update(Args... args) override {
            updater(superclass::graphic, std::forward<Args>(args)...);
        }
    };


}
