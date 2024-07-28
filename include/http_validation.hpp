#ifndef INCLUDE_HTTP_VALIDATION_HPP_
#define INCLUDE_HTTP_VALIDATION_HPP_

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
bool is_relative_uri(const std::string &s);    // = net_path | abs_path | rel_path
bool is_net_path(const std::string &s);       // = "//" net_loc [ abs_path ]
bool is_abs_path(const std::string &s);       // = "/" rel_path
bool is_rel_path(const std::string &s);       // = [ path ] [ ";" params ] [ "?" query ]
bool is_path(const std::string &s);           // = fsegment *( "/" segment )
bool is_fsegment(const std::string &s);       // = 1*pchar
bool is_segment(const std::string &s);        // = *pchar
bool is_params(const std::string &s);         // = param *( ";" param )
bool is_param(const std::string &s);          // = *( pchar | "/" )
bool is_scheme(const std::string &s);         // = 1*( ALPHA | DIGIT | "+" | "-" | "." )
bool is_net_loc(const std::string &s);        // = *( pchar | ";" | "?" )
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

bool is_http_date(const std::string &s);         // HTTP-date      = rfc1123-date | rfc850-date | asctime-date
bool is_rfc1123_data(const std::string &s);      // rfc1123-date   = wkday "," SP date1 SP time SP "GMT"
bool is_rfc850_data(const std::string &s);       // rfc850-date    = weekday "," SP date2 SP time SP "GMT"
bool is_asctime_date(const std::string &s);      // asctime-date   = wkday SP date3 SP time SP 4DIGIT
bool is_date1(const std::string &s);             // date1          = 2DIGIT SP month SP 4DIGIT ; day month year (e.g., 02 Jun 1982)
bool is_date2(const std::string &s);             // date2          = 2DIGIT "-" month "-" 2DIGIT ; day-month-year (e.g., 02-Jun-82)
bool is_date3(const std::string &s);             // date3          = month SP ( 2DIGIT | ( SP 1DIGIT )) ; month day (e.g., Jun  2)
bool is_time(const std::string &s);              // time           = 2DIGIT ":" 2DIGIT ":" 2DIGIT ; 00:00:00 - 23:59:59
bool is_wkday(const std::string &s);             // wkday          = "Mon" | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun"
bool is_weekday(const std::string &s);           // weekday        = "Monday" | "Tuesday" | "Wednesday" | "Thursday" | "Friday" | "Saturday" | "Sunday"
bool is_month(const std::string &s);             // month          = "Jan" | "Feb" | "Mar" | "Apr" | "May" | "Jun" | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"

bool is_pragma_directive(const std::string &s);
bool is_extension_pragma(const std::string &s);

// 補助的な関数 //
bool is_token_element(const char &c);
#define is_text_element(c) (!is_ctl(c) && !is_cr(c) && !is_lf(c))
bool is_target_included_list(std::string target, std::string list[], size_t size);
bool is_4digit(const std::string &s);
bool is_2digit(const std::string &s);

#endif  // INCLUDE_HTTP_VALIDATION_HPP_
