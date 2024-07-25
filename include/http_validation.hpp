#ifndef INCLUDE_HTTP_VALIDATION_HPP_
# define INCLUDE_HTTP_VALIDATION_HPP_

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

bool is_crlf(const std::string &s);
bool is_lws(const std::string &s);
bool is_text(const std::string &s);
bool is_hex(const char &c);
bool is_word(const std::string &s);
bool is_token(const std::string &s);
bool is_tspecials(const char &c);  // こいつをdefineで書くのヤバそうだからとりあえず関数 整合性ないからdefineにしたいという気持ちもある
bool is_comment(const std::string &s);
bool is_ctext(const std::string &s);
bool is_quoted_string(const std::string &s);
bool is_qdtext(const std::string &s);

bool is_valid_http_header(const std::string &str);

// TODO(maitneel): コメントアウトしてるやつ
// ゆるして.............

// bool is_uri(const std::string &s);            // = ( absoluteURI | relativeURI ) [ "#" fragment ]
bool is_absolute_uri(const std::string &s);   // = scheme ":" *( uchar | reserved )
// bool is_relativeURI(const std::string &s);    // = net_path | abs_path | rel_path
// bool is_net_path(const std::string &s);       // = "//" net_loc [ abs_path ]
bool is_abs_path(const std::string &s);       // = "/" rel_path
bool is_rel_path(const std::string &s);       // = [ path ] [ ";" params ] [ "?" query ]
bool is_path(const std::string &s);           // = fsegment *( "/" segment )
bool is_fsegment(const std::string &s);       // = 1*pchar
bool is_segment(const std::string &s);        // = *pchar
bool is_params(const std::string &s);         // = param *( ";" param )
bool is_param(const std::string &s);          // = *( pchar | "/" )
bool is_scheme(const std::string &s);         // = 1*( ALPHA | DIGIT | "+" | "-" | "." )
// bool is_net_loc(const std::string &s);        // = *( pchar | ";" | "?" )
bool is_query(const std::string &s);          // = *( uchar | reserved )
// bool is_fragment(const std::string &s);       // = *( uchar | reserved )
bool is_pchar(const std::string &s);          // = uchar | ":" | "@" | "&" | "=" | "+"
bool is_uchar(const std::string &s);          // = unreserved | escape
bool is_unreserved(const char &c);     // = ALPHA | DIGIT | safe | extra | national
bool is_escape(const std::string &s);         // = "%" HEX HEX

bool is_reserved(const char &c);       // = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+"
bool is_extra(const char &c);          // = "!" | "*" | "'" | "(" | ")" | ","
bool is_safe(const char &c);           // = "$" | "-" | "_" | "."
bool is_unsafe(const char &c);         // = CTL | SP | <"> | "#" | "%" | "<" | ">"
bool is_national(const char &c);       // = <any OCTET excluding ALPHA, DIGIT,     reserved, extra, safe, and unsafe>

bool is_http_version(const std::string &s);  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

bool is_valid_content_length(const std::string &str);

// 補助的な関数 //
bool is_token_element(const char &c);
#define is_text_element(c) (!is_ctl(c) && !is_cr(c) && !is_lf(c))

#endif
