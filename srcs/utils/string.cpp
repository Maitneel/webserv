#include <sys/types.h>
#include <string>
#include <cctype>

void to_lower(std::string *str) {
    for (size_t i = 0; i < str->length(); i++) {
        str->at(i) = tolower(str->at(i));
    }
}

void to_upper(std::string *str) {
    for (size_t i = 0; i < str->length(); i++) {
        str->at(i) = toupper(str->at(i));
    }
}

std::string trim_string(const std::string &origin, const std::string &char_of_remove) {
    size_t front = 0;
    for (size_t i = 0; i < origin.length(); i++) {
        if (front != i) {
            break;
        }
        for (size_t j = 0; j < char_of_remove.length(); j++) {
            if (origin.at(i) == char_of_remove.at(j)) {
                front++;
                break;
            }
        }
    }

    ssize_t back = origin.length();
    for (ssize_t i = origin.length() - 1; 0 <= i ; i--) {
        if (back - 1 != i) {
            break;
        }
        for (size_t j = 0; j < char_of_remove.length(); j++) {
            if (origin.at(i) == char_of_remove.at(j)) {
                back--;
                break;
            }
        }
    }
    return origin.substr(front, back - front);
}

// std::string right_opend_half_open_substr(const std::string &s, const std::string::size_type &closed_left, const std::string::size_type &opend_right) {
//     std::string::size_type length = opend_right - closed_left;
//     return s.substr(closed_left, length);
// }

// std::string left_opend_half_open_substr(const std::string &s, const std::string::size_type &opend_left, const std::string::size_type &closed_right) {
//     std::string::size_type length = closed_right - opend_left;
//     return s.substr(opend_left + 1, length);
// }

// std::string opend_substr(const std::string &s, const std::string::size_type &opend_left, const std::string::size_type &opend_right) {
//     std::string::size_type length = opend_right - opend_left - 1;
//     return s.substr(opend_left + 1, length);
// }

// std::string closed_substr(const std::string &s, const std::string::size_type &closed_left, const std::string::size_type &closed_right) {
//     std::string::size_type length = closed_right - closed_left + 1;
//     return s.substr(closed_left, length);
// }

// #include <iostream>
// #define debug(s) std::cout << #s << ' ' << s << std::endl;
// using std::string;
// int main(int argc, char **argv) {
//     string origin = "aaahogehogebb";
//     debug(trim_string(argv[1], argv[2]));
// }

// #include <iostream>
// using std::cerr;
// using std::endl;
// int main() {
//     std::string s = "hoge:fuga=piyo";
//     cerr << "right open : '" << right_opend_half_open_substr(s, s.find(':'), s.find('=')) << "'" << endl;
//     cerr << "left open  : '" << left_opend_half_open_substr(s, s.find(':'), s.find('=')) << "'" << endl;
//     cerr << "opend      : '" << opend_substr(s, s.find(':'), s.find('=')) << "'" << endl;
//     cerr << "closed     : '" << closed_substr(s, s.find(':'), s.find('=')) << "'" << endl;
// }
