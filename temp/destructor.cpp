#include <iostream>
#include <vector>
using namespace std;

class hoge {
public:
	hoge();
	~hoge();
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
	func();
}