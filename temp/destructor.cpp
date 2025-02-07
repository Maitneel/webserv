#include <iostream>
#include <vector>
using namespace std;

class hoge {
 public :
    hoge();
    ~hoge();
    string str;
};

hoge::hoge() {
    cout << "constructor" << endl;    
}

hoge::~hoge() {
    cout << "destructor" << endl;
}

void func() {
    vector<hoge> a(2);
}

int main() {
    // func();
    hoge h;
    h.str = "hoge";
    cout << h.str << endl;
    h.~hoge();
    cout << h.str << endl;
}