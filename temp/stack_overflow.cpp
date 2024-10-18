#include <iostream>
using namespace std;

int func(int n) {
    cout << n << endl;
    func(n + 1);
    return n;
}

int main() {
    try {
        func(0);
    } catch (exception &e) {
        cerr << e.what() << endl;;
    }
}