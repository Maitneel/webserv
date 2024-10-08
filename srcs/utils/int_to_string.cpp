#include <string>
#include <algorithm>

std::string int_to_string(int n) {
    std::string s;
    while (n) {
        s += n % 10 + '0';
        n /= 10;
    }
    reverse(s.begin(), s.end());
    return s;
}
