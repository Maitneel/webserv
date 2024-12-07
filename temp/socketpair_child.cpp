#include <unistd.h>
#include <iostream>
using namespace std;

int main() {
    // char buf[10];
    // buf[read(STDIN_FILENO, buf, 10)] = 0;
    // cerr << "child: " << buf << endl;
    string str;
    cin >> str;
    cerr << "child: " << str << endl;
    cout << "from_ch" << endl;
}