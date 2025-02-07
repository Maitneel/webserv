#include <iostream>
#include <map>
#include <string>
#include <utility>
using namespace std;

template<typename key, typename value>
value &map_at(std::map<key, value> *m, const key &target) {
    if (m->find(target) == m->end()) {
        throw out_of_range("map_at");
    }
    return m->find(target)->second;
}

int main() {
    std::map<int, int> m1;
    std::map<string, int> m2;

    m1.insert(make_pair(10, 20));
    m2.insert(make_pair(string("hoge"), 30));

    cerr << map_at(&m1, 10) << endl;
    map_at(&m1, 10) = 40;
    cerr << map_at(&m1, 10) << endl;
    cerr << map_at(&m2, string("hoge")) << endl;

}