#include <planet/ecs.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("entities");


    using integral = planet::ecs::storage<bool, int, unsigned>;
    using real = planet::ecs::storage<float, double>;
    using vectors = planet::ecs::storage<std::vector<int>>;

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
        check(e1.mask(0)) == 0u;
        check(e1.mask(1)) == 0u;
        check(e1).is_truthy();

        check(planet::ecs::entity_id{}).is_falsey();
    });


    auto const components = suite.test(
            "components",
            [](auto check) {
                integral int_storage;
                planet::ecs::entities entities{int_storage};

                auto e1 = int_storage.create(42u);

                int_storage.add_component(e1, true);
                check(e1.mask(0)) == 5u;
                int_storage.add_component(e1, 23);
                check(e1.mask(0)) == 7u;

                check(int_storage.get_component<bool>(e1)) == true;
                check(int_storage.get_component<int>(e1)) == 23;
                check(int_storage.get_component<unsigned>(e1)) == 42u;
            },
            [](auto check) {
                integral int_storage;
                vectors vector_storage;
                planet::ecs::entities entities{int_storage, vector_storage};
                auto e1 = entities.create(42u, std::vector{1, 2, 3});
                vector_storage.add_component(e1, std::vector{4, 5, 6});
                entities.add_component(e1, true);
                check(entities.has_component<int>(e1)) == false;
                check(entities.has_component<std::vector<int>>(e1)) == true;
            });


    auto const iteration = suite.test(
            "iteration",
            [](auto check) {
                integral int_storage;
                real real_storage;
                planet::ecs::entities entities{int_storage, real_storage};

                auto e1 = int_storage.create(true, 42u);
                check(e1.mask(0)) == 5u;
                auto e2 = int_storage.create(4, 46u);
                check(e2.mask(0)) == 6u;

                std::size_t count{};
                int_storage.iterate(
                        [&](planet::ecs::entity_id eid, unsigned const &u) {
                            if (count == 0) {
                                check(eid.id()) == 1u;
                                check(u) == 42u;
                            } else if (count == 1) {
                                check(eid.id()) == 2u;
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
                        check(eid.id()) == 2u;
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
                auto e2 = entities.create(6.0f, 42);
                check(entities.get_component<float>(e2)) == 6.0f;
                check(entities.get_component<int>(e2)) == 42;

                std::size_t count{};
                entities.iterate([&](planet::ecs::entity_id eid, unsigned &u,
                                     bool const &b) {
                    check(eid.id()) == 1u;
                    check(u) == 42u;
                    check(b) == true;
                    ++count;
                });
                check(count) == 1u;

                count = 0;
                entities.iterate(
                        [&](planet::ecs::entity_id eid, float &f, bool *b) {
                            if (b) {
                                check(eid) == e1;
                                check(f) == 4.0f;
                                check(*b) == true;
                            } else {
                                check(eid) == e2;
                                check(f) == 6.0f;
                            }
                            ++count;
                        });
                check(count) == 2u;
            },
            [](auto check) {
                integral int_storage;
                real real_storage;
                planet::ecs::entities entities{int_storage, real_storage};

                auto e1 = entities.create(42u, 4.0f);
                auto e2 = entities.create(84u, 6.0f);

                std::size_t count{};
                entities.iterate(
                        [&](planet::ecs::entity_id, unsigned int &u, float &f) {
                            if (count == 0u) {
                                check(u) == 42u;
                                check(f) == 4.0f;
                            } else if (count == 1u) {
                                check(u) == 84u;
                                check(f) == 6.0f;
                            } else {
                                check(true) == false;
                            }
                            ++count;
                        });
                check(count) == 2u;
            },
            [](auto check) {
                integral int_storage;
                real real_storage;
                planet::ecs::entities entities{int_storage, real_storage};

                auto e1 = entities.create(42u, 4.0f);
                auto e2 = entities.create(84u, 6.0f);

                planet::ecs::weak_entity_id we1{e1};
                e1 = {};

                std::size_t count{};
                entities.iterate(
                        [&](planet::ecs::entity_id, unsigned int &u, float &f) {
                            if (count == 0u) {
                                check(u) == 84u;
                                check(f) == 6.0f;
                            } else {
                                check(true) == false;
                            }
                            ++count;
                        });
                check(count) == 1u;

                count = 0;
                e1 = we1.lock();
                entities.iterate(
                        [&](planet::ecs::entity_id, unsigned int &u, float &f) {
                            if (count == 0u) {
                                check(u) == 84u;
                                check(f) == 6.0f;
                            } else {
                                check(true) == false;
                            }
                            ++count;
                        });
                check(count) == 1u;
            });


    auto const remove = suite.test(
            "remove",
            [](auto check) {
                integral int_storage;
                real real_storage;
                planet::ecs::entities entities{int_storage, real_storage};

                auto e1 = entities.create(42u, 4.0f);
                check(int_storage.has_component<int>(e1)) == false;
                check(real_storage.has_component<float>(e1)) == true;

                int_storage.remove_component<int>(e1);
                check(int_storage.has_component<int>(e1)) == false;

                real_storage.remove_component<float>(e1);
                check(real_storage.has_component<float>(e1)) == false;
            },
            [](auto check) {
                integral int_storage;
                vectors vector_storage;
                planet::ecs::entities entities{int_storage, vector_storage};
                auto e1 = entities.create(42u, std::vector{1, 2, 3});
                vector_storage.remove_component<std::vector<int>>(e1);
                check(entities.maybe_get_component<std::vector<int>>(e1))
                        == nullptr;
            },
            [](auto check) {
                integral int_storage;
                vectors vector_storage;
                planet::ecs::entities entities{int_storage, vector_storage};
                auto e1 = entities.create(42u, std::vector{1, 2, 3});
                check(entities.is_valid(e1)) == true;
                check(e1).is_truthy();
                entities.kill(e1);
                check(entities.is_valid(e1)) == false;

                auto e2 = entities.create();
                check(e2.id()) == e1.id();
                check(e1) != e2;
            },
            [](auto check) {
                integral int_storage;
                vectors vector_storage;
                planet::ecs::entities entities{int_storage, vector_storage};
                auto e1 = entities.create(42u, std::vector{1, 2, 3});
                entities.clear();
                check(entities.is_valid(e1)) == false;
            });


    auto const proxy = suite.test("component_proxy", [](auto check) {
        integral int_storage;
        real real_storage;
        planet::ecs::entities entities{int_storage, real_storage};

        auto e1 = entities.create();
        auto p1 = entities.add_component(e1, true);
        check(entities.has_component<bool>(e1)) == true;

        p1.remove();
        check(entities.has_component<bool>(e1)) == false;
    });


}
