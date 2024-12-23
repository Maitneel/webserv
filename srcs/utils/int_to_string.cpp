#include <string>
#include <algorithm>

std::string int_to_string(int n) {
    std::string s;
    bool is_negative = (n < 0);
    while (n) {
        s += n % 10 + '0';
        n /= 10;
    }
    if (is_negative) {
        s += "-";
    }
    reverse(s.begin(), s.end());
    return s;
}

std::string size_t_to_string(size_t n) {
    std::string s;
    while (n) {
        s += n % 10 + '0';
        n /= 10;
    }
    reverse(s.begin(), s.end());
    return s;
}

const std::string hex_str = "0123456789ABCDEF";

std::string size_t_to_hex_string(size_t n) {
    std::string s;
    while (n) {
        s += hex_str[n % 16];
        n /= 16;
    }
    reverse(s.begin(), s.end());
    return s;
}
