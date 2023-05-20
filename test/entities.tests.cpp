#include <planet/ecs.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("entities");


    using integral = planet::ecs::storage<bool, int, unsigned>;
    using real = planet::ecs::storage<float, double>;

    static_assert(not integral::maybe_component_index<float>().has_value());
    static_assert(integral::maybe_component_index<bool>().value() == 0);
    static_assert(integral::maybe_component_index<int>().value() == 1);
    static_assert(integral::maybe_component_index<unsigned>().value() == 2);

    static_assert(not planet::ecs::detail::same_type_index<int, unsigned>(0)
                              .has_value());
    static_assert(
            planet::ecs::detail::same_type_index<int, int>(0).value() == 0);

    using namespace planet::ecs::detail;
    static_assert(not(type_index{} || type_index{}).has_value());
    static_assert((type_index{1} || type_index{}).value() == 1);
    static_assert((type_index{} || type_index{2}).value() == 2);
    // Fails to compile, which is what we want
    // static_assert((type_index{1} || type_index{2}).value() == 2);


    auto const create = suite.test("create", [](auto check) {
        integral int_storage;
        real real_storage;
        planet::ecs::entities entities{int_storage, real_storage};

        auto e1 = entities.create();
        check(e1->components.size()) == 2u;
        check(e1->components[0]) == 0u;
        check(e1->components[1]) == 0u;
    });


    auto const components = suite.test("components", [](auto check) {
        integral int_storage;
        planet::ecs::entities entities{int_storage};

        auto e1 = int_storage.create(42u);

        int_storage.add_component(e1, true);
        check(e1->components[0]) == 5u;
        int_storage.add_component(e1, 23);
        check(e1->components[0]) == 7u;

        check(int_storage.get_component<bool>(e1)) == true;
        check(int_storage.get_component<int>(e1)) == 23;
        check(int_storage.get_component<unsigned>(e1)) == 42u;
    });


    auto const iteration = suite.test(
            "iteration",
            [](auto check) {
                integral int_storage;
                real real_storage;
                planet::ecs::entities entities{int_storage, real_storage};

                auto e1 = int_storage.create(true, 42u);
                check(e1->components[0]) == 5u;
                auto e2 = int_storage.create(4, 46u);
                check(e2->components[0]) == 6u;

                std::size_t count{};
                int_storage.iterate(
                        [&](planet::ecs::entity_id eid, unsigned const &u) {
                            if (count == 0) {
                                check(eid.id) == 1u;
                                check(u) == 42u;
                            } else if (count == 1) {
                                check(eid.id) == 2u;
                                check(u) == 46u;
                            } else {
                                check(false) == true;
                            }
                            ++count;
                        });
                check(count) == 2u;

                count = {};
                int_storage.iterate([&](planet::ecs::entity_id eid, int &i) {
                    if (count == 0) {
                        check(eid.id) == 2u;
                        check(i) == 4;
                    } else {
                        check(false) == true;
                    }
                    ++count;
                });
                check(count) == 1u;
            },
            [](auto check) {
                integral int_storage;
                real real_storage;
                planet::ecs::entities entities{int_storage, real_storage};

                auto e1 = entities.create(true, 42u, 4.0f);
                check(entities.get_component<bool>(e1)) == true;
                check(entities.get_component<unsigned>(e1)) == 42u;
                check(entities.get_component<float>(e1)) == 4.0f;

                std::size_t count{};
                entities.iterate([&](planet::ecs::entity_id eid, unsigned &u,
                                     bool const &b) {
                    check(eid.id) == 1u;
                    check(u) == 42u;
                    check(b) == true;
                    ++count;
                });
                check(count) == 1u;
            });


}
