#include <iostream>

#include "simple_db.hpp"
using namespace std;

SimpleDB db("./temp.db");

int main() {
    string key, value;
    cout << "key: " << flush;
    while (cin >> key) {
        cout << "value: ";
        cin >> value;
        try {
            db.add(key, value);
        } catch (exception &e) {
            cerr << e.what() << endl;
        }
        cout << "key: ";
    }

    cout << "---------------------------------------" << endl;
    vector<string> key_list = db.get_include_key();
    for (size_t i = 0; i < key_list.size(); i++) {
        cout << "'" << key_list.at(i) << "': '" << db.get(key_list.at(i)) << "'" << endl;
    }
    
    return 0;
}
