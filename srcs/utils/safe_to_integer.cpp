#include <string>
#include <stdexcept>
#include <cctype>
#include <climits>

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
                throw std::runtime_error("safe_atoi: minus overflow");
            }
        } else {
            if (result < INT_MAX / 10 || (result == INT_MAX / 10 && str.at(str_index) - '0' <= INT_MAX % 10)) {
                result *= 10;
                result += str.at(str_index) - '0';
            } else {
                throw std::runtime_error("safe_atoi: overflow");
            }
        }
    }
    return result;
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
