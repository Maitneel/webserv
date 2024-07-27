#include <iostream>
#include <vector>
#include <utility>
using namespace std;

class Hoge {
private: 
    void func1() {cout << "func1" << endl;}
    void func2() {cout << "func2" << endl;}

    void(Hoge::*pointer)();
public:
    Hoge();
    void callfunc();
};

void Hoge::callfunc() {
    (this->*pointer)();
}

Hoge::Hoge() {
    void(Hoge::*func)() = &Hoge::func1;
    pair<int, void(Hoge::*)()> aaa(10, &Hoge::func2);
    pointer = &Hoge::func1;
    (this->*pointer)();
    (this->*func)();
    // func = aaa->second;
    (this->*(aaa.second))();

}

int main() {
    Hoge fuga;
    // fuga.callfunc();
    return 0;
}