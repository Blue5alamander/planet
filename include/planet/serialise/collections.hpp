#pragma once


#include <map>
#include <set>


namespace planet::serialise {


    /// ## Serialise `std::map`
    template<typename K, typename V, typename C, typename A>
    void save(save_buffer &ab, std::map<K, V, C, A> const &m) {
        ab.save_box_lambda(2, "_s:map", [&]() {
            save(ab, m.size());
            for (auto const &[k, v] : m) {
                save(ab, k);
                save(ab, v);
            }
        });
    }
    template<typename K, typename V, typename C, typename A>
    void load(load_buffer &lb, std::map<K, V, C, A> &m) {
        m.clear();
        auto box = expect_box(lb);
        box.check_name_or_throw("_s:map");
        auto const count = [&]() {
            if (box.version == 2) {
                return load_type<std::size_t>(box.content);
            } else if (box.version == 1) {
                return box.content.extract_size_t();
            } else {
                box.throw_unsupported_version(2);
            }
        }();
        while (m.size() < count) {
            load(box.content, m[load_type<K>(box.content)]);
        }
        box.check_empty_or_throw();
    }


    /// ## Serialise `std::set`
    template<typename K, typename C, typename A>
    void save(save_buffer &ab, std::set<K, C, A> const &m) {
        ab.save_box_lambda(1, "_s:set", [&]() {
            save(ab, m.size());
            for (auto const &k : m) { save(ab, k); }
        });
    }
    template<typename K, typename C, typename A>
    void load(load_buffer &lb, std::set<K, C, A> &m) {
        m.clear();
        auto box = expect_box(lb);
        box.check_name_or_throw("_s:set");
        auto const count = load_type<std::size_t>(box.content);
        while (m.size() < count) { m.insert(load_type<K>(box.content)); }
        box.check_empty_or_throw();
    }


}
