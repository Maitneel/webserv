#include <iostream>
#include <string>
using namespace std;

int main(int argc, char **argv) {
    string str;
    string exc;
    cin >> exc;
    cerr << exc << endl;
    for (size_t i = ' '; i < 128; i++) {
        bool include = false;
        for (size_t j = 0; j < exc.length(); j++) {
            if (i == exc.at(j)) {
                include = true;
                break;
            }
        }
        if (!include) {
            cout << (char)(i);
        }
    }
    cout << endl;
}