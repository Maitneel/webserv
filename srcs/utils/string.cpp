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

// #include <iostream>
// #define debug(s) std::cout << #s << ' ' << s << std::endl;
// using std::string;
// int main(int argc, char **argv) {
//     string origin = "aaahogehogebb";
//     debug(trim_string(argv[1], argv[2]));
// }
