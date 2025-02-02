#include <string>
#include <stdexcept>
#include <cctype>
#include <climits>
#include <stdint.h>

// 基本的な動作は stdclib の atoi と同じだが、　int の範囲外の値を変換しようとした際に exception を投げる //
int safe_atoi(std::string str) {
    if (str.length() == 0) {
        return 0;
    }
    bool is_negative = false;
    size_t str_index = 0;
    if (str.at(0) == '-') {
        is_negative = true;
        str_index++;
    }

    int result = 0;
    for (; str_index < str.length() && isdigit(str.at(str_index)); str_index++) {
        if (is_negative) {
            if (INT_MIN / 10 < result || (result == INT_MIN / 10 && str.at(str_index) - '0' <= INT_MAX % 10 * -1)) {
                result *= 10;
                result -= str.at(str_index) - '0';
            } else {
                throw std::overflow_error("safe_atoi: minus overflow");
            }
        } else {
            if (result < INT_MAX / 10 || (result == INT_MAX / 10 && str.at(str_index) - '0' <= INT_MAX % 10)) {
                result *= 10;
                result += str.at(str_index) - '0';
            } else {
                throw std::overflow_error("safe_atoi: overflow");
            }
        }
    }
    return result;
}

size_t safe_hex_to_sizet(const std::string &str) {
    size_t front = 0;
    size_t n = 0;
    for (front = 0; front < str.length(); front++) {
        char current = toupper(str.at(front));
        if (!(isdigit(current) || ('A' <= current && current <= 'F'))) {
            break;
        }
        if (SIZE_MAX / 0x10 < n) {
            throw std::overflow_error("safe_hex_to_sizet: overflow");
        }
        n *= 0x10;
        if (isdigit(current)) {
            n += current - '0';
        } else {
            n += 10 + current - 'A';
        }
    }
    if (front == 0) {
        throw std::runtime_error("safe_hex_to_sizet: not hex number");
    }
    return n;
}

// #include <iostream>
// #include <string>
// #include <vector>
// using namespace std;
// #define debug(s) std::cout << #s << ' ' << s << std::endl;
// int main() {
//     vector<string> str = {to_string(INT_MAX), to_string((long long)(INT_MAX) + 1), to_string(INT_MIN),
//                           to_string((long long )(INT_MIN) + 1)};
//     for (size_t i = 0; i < str.size(); i++) {
//         try {
//             std::cout << str.at(i) << ' ';
//             debug(safe_atoi(str.at(i)));
//         } catch(const std::exception& e) {
//             std::cerr << e.what() << endl;
//         }
//     }
// }

// #include <iostream>
// #include <iomanip>
// using namespace std;
// int main(int argc, char **argv) {
//     for (int i = 1; i < argc; i++) {
//         try {
//             cout << setw(20) << argv[i] << ": " << safe_hex_to_sizet(argv[i]) << endl;
//         } catch (std::exception &e) {
//             cerr << e.what() << endl;
//         }
//     }
//     cerr << SIZE_MAX << endl;
// }
