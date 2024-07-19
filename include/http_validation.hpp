#ifndef KEYWORDS_HPP
# define KEYWORDS_HPP

#include <cctype>
#include <string>

#define CR 13 
#define LF 10

// 関数にしてもいいけど、そこまで複雑じゃないし、こっちの方が処理的に良い気がするからとりあえずこうしてる
#define is_char(c) (0 <= (c) && (c) <= 128)
#define is_upper(c) isupper(c)
#define is_lower(c) islower(c)
#define is_alpha(c) (is_upper(c) || is_lower(c))
#define is_digit(c)  isdigit(c)
#define is_ctl(c) ((0 <= (c) && (c) <= 31) || (c) == 127)
#define is_cr(c) ((c) == CR)
#define is_lf(c) ((c) == LF)
#define is_sp(c) ((c) == 32)
#define is_ht(c) ((c) == 9)
#define is_dquote(c) ((c) == 34)

bool is_crlf(std::string s);
bool is_lws(std::string s);
bool is_text(std::string s);
bool is_hex(char c);
bool is_word(std::string s);
bool is_token(std::string s);
bool is_tspecials(char c); // こいつをdefineで書くのヤバそうだからとりあえず関数 整合性ないからdefineにしたいという気持ちもある
bool is_comment(std::string s);
bool is_ctext(std::string s);
bool is_quoted_string(std::string s);
bool is_qdtext(std::string s);


// TODO コメントアウトしてるやつ
// ゆるして.............

// bool is_uri(std::string s);            // = ( absoluteURI | relativeURI ) [ "#" fragment ]
bool is_absolute_uri(std::string s);   // = scheme ":" *( uchar | reserved )
// bool is_relativeURI(std::string s);    // = net_path | abs_path | rel_path
// bool is_net_path(std::string s);       // = "//" net_loc [ abs_path ]
bool is_abs_path(std::string s);       // = "/" rel_path
bool is_rel_path(std::string s);       // = [ path ] [ ";" params ] [ "?" query ]
bool is_path(std::string s);           // = fsegment *( "/" segment )
bool is_fsegment(std::string s);       // = 1*pchar
bool is_segment(std::string s);        // = *pchar
bool is_params(std::string s);         // = param *( ";" param )
bool is_param(std::string s);          // = *( pchar | "/" )
bool is_scheme(std::string s);         // = 1*( ALPHA | DIGIT | "+" | "-" | "." )
// bool is_net_loc(std::string s);        // = *( pchar | ";" | "?" )
bool is_query(std::string s);          // = *( uchar | reserved )
// bool is_fragment(std::string s);       // = *( uchar | reserved )
bool is_pchar(std::string s);          // = uchar | ":" | "@" | "&" | "=" | "+"
bool is_uchar(std::string s);          // = unreserved | escape
bool is_unreserved(char c);     // = ALPHA | DIGIT | safe | extra | national
bool is_escape(std::string s);         // = "%" HEX HEX

bool is_reserved(char c);       // = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+"
bool is_extra(char c);          // = "!" | "*" | "'" | "(" | ")" | ","
bool is_safe(char c);           // = "$" | "-" | "_" | "."
bool is_unsafe(char c);         // = CTL | SP | <"> | "#" | "%" | "<" | ">"
bool is_national(char c);       // = <any OCTET excluding ALPHA, DIGIT,     reserved, extra, safe, and unsafe>

bool is_http_version(std::string s);  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

// 補助的な関数 //
bool is_token_element(char c);

// getter 
std::string get_first_token(std::string s);

#endif
