#include <string>
#include <vector>
#include <cstdlib>


std::string gen_random_str(const size_t length = 8) {
    const std::string useable_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz_";
    srand((unsigned int)time(NULL));

    std::string random_str;
    for (size_t i = 0; i < length; i++) {
        int random_number = rand();
        random_str += useable_char.at(random_number % useable_char.length());
    }
    return random_str;
}

std::vector<std::string> escaped_quote_split(const std::string &src, const std::string &delimiter) {
    std::vector<std::string> result;
    std::string::size_type front = 0;
    std::string::size_type back = 0;
    std::string::size_type quote_start = src.find('"');
    std::string::size_type quote_end = src.find('"', quote_start + 1);

    if (delimiter.length() == 0) {
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

std::string trim(const std::string &src, const std::string &remove_char) {
    try {
        std::string::size_type front = src.find_first_not_of(remove_char);
        std::string::size_type back = src.find_last_not_of(remove_char) + 1;
        return src.substr(front, back - front);
    } catch (...) {
    }
    return "";
}