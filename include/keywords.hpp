#ifndef KEYWORDS_HPP
# define KEYWORDS_HPP

#include <cctype>
#include <string>

// 関数にしてもいいけど、そこまで複雑じゃないし、こっちの方が処理的に良い気がするからとりあえずこうしてる
#define is_char(c) (0 <= (c) && (c) <= 128)
#define is_upper(c) isupper(c)
#define is_lower(c) islower(c)
#define is_alpha(c) (is_upper(c) || is_lower(c))
#define is_digit(c) isdigit(c)
#define is_ctl(c) ((0 <= (c) && (c) <= 31) || (c) == 127)
#define is_cr(c) ((c) == 13)
#define is_lf(c) ((c) == 10)
#define is_sp(c) ((c) == 32)
#define is_ht(c) ((c) == 9)
#define is_dquote(c) ((c) == 34)

bool is_crlf(std::string s);
bool is_lws(std::string s);
bool is_text(std::string s);
bool is_hex(std::string s);
bool is_word(std::string s);
bool is_token(std::string s);
bool is_tspecials(char c); // こいつをdefineで書くのヤバそうだからとりあえず関数 整合性ないからdefineにしたいという気持ちもある
bool is_comment(std::string s);
bool is_ctext(std::string s);
bool is_quoted_string(std::string s);
bool is_qdtext(std::string s);

#endif
