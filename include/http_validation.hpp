#ifndef INCLUDE_HTTP_VALIDATION_HPP_
#define INCLUDE_HTTP_VALIDATION_HPP_

#include <cctype>
#include <string>

#define CR 0x0d
#define LF 0x0a

#define CRLF "\x0d\x0a"

#define SP 0x20
#define HTAB 0x09


// BIT2
// HEXDIG
// LWSP
// WSP
// OCTET
// VCHAR

// 関数にしてもいいけど、そこまで複雑じゃないし、こっちの方が処理的に良い気がするからとりあえずこうしてる
// CHAR は RFC1945 では　　0x00, 0x80 を含むが、RFC5234 では含まない？（本当か？） //
#define is_char(c) (0x01 <= (c) && (c) <= 0x7f)
// UPCHAR, LOCHAR は RFC5234 では定義されていない
#define is_upper(c) isupper(c)
#define is_lower(c) islower(c)
#define is_alpha(c) (is_upper(c) || is_lower(c))
#define is_digit(c)  isdigit(c)
#define is_ctl(c) ((0x00 <= (c) && (c) <= 0x1f) || (c) == 0x7f)
#define is_cr(c) ((c) == CR)
#define is_lf(c) ((c) == LF)
#define is_sp(c) ((c) == 0x20)
#define is_ht(c) ((c) == 0x09)
#define is_dquote(c) ((c) == 0x22)

#define is_vchar(c) (0x21 <= c && c <= 0x7e)
#define is_obs_text(c) (0x80 <= c && c <= 0xff)


bool is_crlf(const std::string &s);
bool is_lws(const std::string &s);  // unchecked http/1.1
bool is_text(const std::string &s);  // unchecked http/1.1
bool is_hex(const char &c);  // unchecked http/1.1
bool is_word(const std::string &s);  // unchecked http/1.1
bool is_token(const std::string &s);
bool is_tspecials(const char &c);  // こいつをdefineで書くのヤバそうだからとりあえず関数 整合性ないからdefineにしたいという気持ちもある  // unchecked http/1.1
bool is_comment(const std::string &s);  // 厳密にいうとちょっと違う(quoted-pairの処理をしてない) //
bool is_ctext(const std::string &s);
bool is_quoted_string(const std::string &s);  // 厳密にいうとちょっと違う(quoted-pairの処理をしてない) //
bool is_qdtext(const std::string &s);

bool is_valid_http_header(const std::string &str);  // unchecked http/1.1

// TODO(maitneel): コメントアウトしてるやつ
// ゆるして.............

// bool is_uri(const std::string &s);            // = ( absoluteURI | relativeURI ) [ "#" fragment ]  // unchecked http/1.1
bool is_absolute_uri(const std::string &s);   // = scheme ":" *( uchar | reserved )  // unchecked http/1.1
bool is_relative_uri(const std::string &s);    // = net_path | abs_path | rel_path  // unchecked http/1.1
bool is_net_path(const std::string &s);       // = "//" net_loc [ abs_path ]  // unchecked http/1.1
bool is_abs_path(const std::string &s);       // = "/" rel_path  // unchecked http/1.1
bool is_rel_path(const std::string &s);       // = [ path ] [ ";" params ] [ "?" query ]  // unchecked http/1.1
bool is_path(const std::string &s);           // = fsegment *( "/" segment )  // unchecked http/1.1
bool is_fsegment(const std::string &s);       // = 1*pchar  // unchecked http/1.1
bool is_segment(const std::string &s);        // = *pchar  // unchecked http/1.1
bool is_params(const std::string &s);         // = param *( ";" param )  // unchecked http/1.1
bool is_param(const std::string &s);          // = *( pchar | "/" )  // unchecked http/1.1
bool is_scheme(const std::string &s);         // = 1*( ALPHA | DIGIT | "+" | "-" | "." )  // unchecked http/1.1
bool is_net_loc(const std::string &s);        // = *( pchar | ";" | "?" )  // unchecked http/1.1
bool is_query(const std::string &s);          // = *( uchar | reserved )  // unchecked http/1.1
// bool is_fragment(const std::string &s);       // = *( uchar | reserved )  // unchecked http/1.1
bool is_pchar(const std::string &s);          // = uchar | ":" | "@" | "&" | "=" | "+"  // unchecked http/1.1
bool is_uchar(const std::string &s);          // = unreserved | escape  // unchecked http/1.1
bool is_unreserved(const char &c);     // = ALPHA | DIGIT | safe | extra | national  // unchecked http/1.1
bool is_escape(const std::string &s);         // = "%" HEX HEX  // unchecked http/1.1

bool is_reserved(const char &c);       // = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+"  // unchecked http/1.1
bool is_extra(const char &c);          // = "!" | "*" | "'" | "(" | ")" | ","  // unchecked http/1.1
bool is_safe(const char &c);           // = "$" | "-" | "_" | "."  // unchecked http/1.1
bool is_unsafe(const char &c);         // = CTL | SP | <"> | "#" | "%" | "<" | ">"  // unchecked http/1.1
bool is_national(const char &c);       // = <any OCTET excluding ALPHA, DIGIT,     reserved, extra, safe, and unsafe>  // unchecked http/1.1

bool is_http_version(const std::string &s);  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT  // unchecked http/1.1

bool is_http_date(const std::string &s);         // HTTP-date      = rfc1123-date | rfc850-date | asctime-date
bool is_rfc1123_data(const std::string &s);      // rfc1123-date aka IMT date   = wkday "," SP date1 SP time SP "GMT"
bool is_rfc850_data(const std::string &s);       // rfc850-date    = weekday "," SP date2 SP time SP "GMT" 現在から過去50年と未来50年として扱う
bool is_asctime_date(const std::string &s);      // asctime-date   = wkday SP date3 SP time SP 4DIGIT
bool is_date1(const std::string &s);             // date1          = 2DIGIT SP month SP 4DIGIT ; day month year (e.g., 02 Jun 1982)
bool is_date2(const std::string &s);             // date2          = 2DIGIT "-" month "-" 2DIGIT ; day-month-year (e.g., 02-Jun-82)
bool is_date3(const std::string &s);             // date3          = month SP ( 2DIGIT | ( SP 1DIGIT )) ; month day (e.g., Jun  2)
bool is_time(const std::string &s);              // time           = 2DIGIT ":" 2DIGIT ":" 2DIGIT ; 00:00:00 - 23:59:59
bool is_wkday(const std::string &s);             // wkday          = "Mon" | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun"
bool is_weekday(const std::string &s);           // weekday        = "Monday" | "Tuesday" | "Wednesday" | "Thursday" | "Friday" | "Saturday" | "Sunday"
bool is_month(const std::string &s);             // month          = "Jan" | "Feb" | "Mar" | "Apr" | "May" | "Jun" | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"

bool is_pragma_directive(const std::string &s);  // unchecked http/1.1
bool is_extension_pragma(const std::string &s);  // unchecked http/1.1

bool is_product(const std::string &s);  // unchecked http/1.1

// 補助的な関数 //
bool is_token_element(const char &c);
#define is_text_element(c) (!is_ctl(c) && !is_cr(c) && !is_lf(c))
bool is_target_included_list(const std::string target, const std::string list[], size_t size);
bool is_4digit(const std::string &s);
bool is_2digit(const std::string &s);
#define is_filed_vchar_element(c) (is_vchar(c) || is_obs_text(c))
#define is_ows_elment(c) (c == SP || c == HTAB)

#endif  // INCLUDE_HTTP_VALIDATION_HPP_
