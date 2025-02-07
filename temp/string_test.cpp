#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
using namespace std;

void gen_str(string *str, int len) {
    for (int i = 0; i < len; i++) {
        str->push_back('0' + (i % 10));
    }
}

void str_erase(std::string str, int len) {
    while (str.length()) {
        str.erase(0, len);
    }
}

int main(int argc, char **argv) {
    string str;
    gen_str(&str, atoi(argv[1]));
    clock_t start1 = clock();
    str_erase(str, 100);
    clock_t end1 = clock();
    clock_t start2 = clock();
    str_erase(str, 10000);
    clock_t end2 = clock();
    clock_t start3 = clock();
    str_erase(str, 1000000);
    clock_t end3 = clock();

    cout << "erase size     100: " << end1 - start1 << endl;
    cout << "erase size   10000: " << end2 - start2 << endl;
    cout << "erase size 1000000: " << end3 - start3 << endl;

}