#include <fstream>
#include <iostream>
using namespace std;

int main()  {
    ifstream ifs;
    ifs.setstate(ios_base::failbit);
    cerr << (bool)(ifs) << endl;
    ifs.open("Makefile");
    cerr << (bool)(ifs) << endl;
    return 0;
}

