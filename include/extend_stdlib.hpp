#ifndef INCLUDE_EXTEND_STDLIB_HPP_
#define INCLUDE_EXTEND_STDLIB_HPP_

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

std::vector<std::string> split(const std::string &src, const std::string &delimiter);
std::vector<std::string> escaped_quote_split(const std::string &src, const std::string &delimiter);
std::vector<std::string> split_with_remove(const std::string &src, const std::string &delimiter);
int safe_atoi(std::string str);
size_t safe_hex_to_sizet(const std::string &str);
std::string get_formated_date();
void to_lower(std::string *str);
void to_upper(std::string *str);
std::string trim_string(const std::string &origin, const std::string &char_of_remove);
std::string int_to_string(int n);
std::string size_t_to_string(size_t n);
std::string size_t_to_hex_string(size_t n);


// 関数にしたくない(この処理でスタック積みたくない)のでマクロにしてる //
// srcs/utils/string.cpp　にコメントアウトした実装あるので処理わからなければ見てもらえるといいと思う //

// std::string left_opend_half_open_substr(const std::string &s, const std::string::size_type &opend_left, const std::string::size_type &closed_right);
#define right_opend_half_open_substr(s, closed_left, opend_right) (s).substr((closed_left), (opend_right) - (closed_left))
// std::string right_opend_half_open_substr(const std::string &s, const std::string::size_type &closed_left, const std::string::size_type &opend_right);
#define left_opend_half_open_substr(s, opend_left, closed_right) (s).substr(((opend_left) + 1), (closed_right) - (opend_left))
// std::string opend_substr(const std::string &s, const std::string::size_type &opend_left, const std::string::size_type &opend_right);
#define opend_substr(s, opend_left, opend_right) (s).substr(((opend_left) + 1), (opend_right) - (opend_left) - 1)
// std::string closed_substr(const std::string &s, const std::string::size_type &closed_left, const std::string::size_type &closed_right);
#define closed_substr(s, closed_left, closed_right) (s).substr((closed_left), (closed_right) - (closed_left) + 1)

// template<typename key, typename value>
// value &map_at(std::map<key, value> *m, const key &target);

template<typename key, typename value>
value &map_at(std::map<key, value> *m, const key &target) {
    if (m->find(target) == m->end()) {
        throw std::out_of_range("map_at");
    }
    return m->find(target)->second;
}


#endif  // INCLUDE_EXTEND_STDLIB_HPP_
