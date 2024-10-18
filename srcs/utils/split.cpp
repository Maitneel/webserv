#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> split(const std::string &src, const std::string &delimiter) {
    std::vector<std::string> result;
    std::string::size_type front = 0;
    std::string::size_type back = 0;

    if (delimiter.length() == 0) {
        // ここどうする? //
        result.push_back(src);
        return result;
    }
    do {
        front = back;
        back = src.find(delimiter, front);
        if (back != std::string::npos) {
            back += delimiter.length();
        }
        result.push_back(src.substr(front, back - front));
    } while (back != std::string::npos && back != src.length());
    return result;
}

std::vector<std::string> escaped_quote_split(const std::string &src, const std::string &delimiter) {
    std::vector<std::string> result;
    std::string::size_type front = 0;
    std::string::size_type back = 0;
    std::string::size_type quote_start = src.find('"');
    std::string::size_type quote_end = src.find('"', quote_start + 1);

    if (delimiter.length() == 0) {
        // ここどうする? //
        result.push_back(src);
        return result;
    }
    do {
        front = back;
        do {
            std::string::size_type search_front = back;
            back = src.find(delimiter, search_front);
            if (back != std::string::npos) {
                back += delimiter.length();
            }
        } while ((front <= quote_start && quote_start < back) && back <= quote_end && quote_end != std::string::npos);
        result.push_back(src.substr(front, back - front));
    } while (back != std::string::npos && back != src.length());
    return result;
}

std::vector<std::string> split_with_remove(const std::string &src, const std::string &delimiter) {
    std::vector<std::string> result;
    std::string::size_type front = 0;
    std::string::size_type back = 0;

    if (delimiter.length() == 0) {
        return result;
    }
    do {
        front = back;
        back = src.find(delimiter, front);
        if (back != std::string::npos) {
            back += delimiter.length();
        }
        std::string splited_substr = src.substr(front, back - front - 1);
        if (splited_substr.length() != 0) {
            result.push_back(splited_substr);
        }
    } while (back != std::string::npos && back != src.length());
    return result;
}

// int main() {
//     std::string hoge = "ab\"ababhoge\"hogeab";
//     std::vector<std::string> sp = escaped_quote_split(hoge, "ab");
//     for (size_t i = 0; i < sp.size(); i++) {
//         std::cout << "'" << sp[i] << "'" << std::endl;
//     }
//    return 0;
// }

// int main(const int argc, const char **argv) {
//     std::string str = argv[1];
//     std::string del = argv[2];
//     std::vector<std::string> splited = split_with_remove(str, del);
//     for (size_t i = 0; i < splited.size(); i++) {
//         std::cerr << i << " : " << splited[i] << std::endl;;
//     }
// }
