#pragma once


#include <planet/affine/extents2d.hpp>
#include <planet/ui/reflowable.hpp>
#include <planet/ui/scale.hpp>


namespace planet::ui {


    /// ## Draw re-sizeable content at an aspect ratio locked size
    template<typename C>
    struct target_aspect final : public reflowable {
        using content_type = C;


        target_aspect(content_type c, affine::extents2d const a)
            requires(not std::is_lvalue_reference_v<content_type>)
        : reflowable{"planet::ui::target_aspect"},
          content{std::move(c)},
          aspect{a} {}
        target_aspect(content_type c, affine::extents2d const a)
            requires std::is_lvalue_reference_v<content_type>
        : reflowable{"planet::ui::target_aspect"}, content{c}, aspect{a} {}


        content_type content;
        affine::extents2d aspect;


        void draw() { content.draw(); }


      private:
        affine::extents2d computed{};

        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &c) override {
            auto const size = scaling(aspect, c, scale::lock_aspect);
            computed = size.extents();
            content.reflow(p, size);
            return size;
        }

        affine::rectangle2d move_sub_elements(
                reflow_parameters const &p,
                affine::rectangle2d const &r) override {
            content.move_to(p, {r.top_left, computed});
            return {r.top_left, computed};
        }
    };

    /// ### Aspect ratio spacer with no content
    template<>
    struct target_aspect<void> final : public reflowable {
        using content_type = void;


        target_aspect(affine::extents2d const a)
        : reflowable{"planet::ui::target_aspect"}, aspect{a} {}


        affine::extents2d aspect;


        void draw() {}


      private:
        affine::extents2d computed{};

        constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &c) override {
            auto const size = scaling(aspect, c, scale::lock_aspect);
            computed = size.extents();
            return size;
        }

        affine::rectangle2d move_sub_elements(
                reflow_parameters const &,
                affine::rectangle2d const &r) override {
            return {r.top_left, computed};
        }
    };


    target_aspect(affine::extents2d) -> target_aspect<void>;

    template<typename C>
    target_aspect(C, affine::extents2d) -> target_aspect<C>;


}
