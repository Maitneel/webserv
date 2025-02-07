#include <map>
#include <stdexcept>

template<typename key, typename value>
value &map_at(std::map<key, value> *m, const key &target) {
    if (m->find(target) == m->end()) {
        throw std::out_of_range("map_at");
    }
    return m->find(target)->second;
}
