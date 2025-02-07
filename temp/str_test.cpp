#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

int main() {
    string str = "0123456789abcdefghijklmnopqrstuvwxyz";
    write(STDOUT_FILENO, str.c_str(), str.length());
    cout << endl;
    write(STDOUT_FILENO, str.c_str() + 10, str.length() - 10);
    cout << endl;


}