#pragma once


#include <map>


namespace planet::serialise {


    template<typename K, typename V>
    void save(save_buffer &ab, std::map<K, V> const &m) {
        ab.save_box_lambda("_s:map", [&]() {
            ab.append_size_t(m.size());
            for (auto const &[k, v] : m) {
                save(ab, k);
                save(ab, v);
            }
        });
    }
    template<typename K, typename V>
    void load(load_buffer &lb, std::map<K, V> &m) {
        auto box = load_type<serialise::box>(lb);
        box.check_name_or_throw("_s:map");
        auto const count = box.content.extract_size_t();
        while (m.size() < count) {
            load(box.content, m[load_type<K>(box.content)]);
        }
        box.check_empty_or_throw();
    }


}
