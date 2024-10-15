#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "http_validation.hpp"
#include "get_http_keyword.hpp"
#include "extend_stdlib.hpp"

bool is_crlf(const std::string &s) {
    if (s.length() != 2) {
        return false;
    }
    return (is_cr(s.at(0)) && is_lf(s.at(1)));
}

bool is_lws(const std::string &s) {
    if (s.length() < 3) {
        return false;
    }
    if (!is_crlf(s.substr(0, 2))) {
        return false;
    }
    for (size_t i = 2; i < s.length(); i++) {
        if (!(is_sp(s.at(i)) || is_ht(s.at(i)))) {
            return false;
        }
    }
    return true;
}

bool is_text(const std::string &s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (is_ctl(s.at(i))) {
            return false;
        }
    }
    return true;
}

bool is_hex(const char &c) {
    return (
        ('A' <= c && c <= 'F') ||
        ('a' <= c && c <= 'f') ||
        is_digit(c)
    );
}

bool is_word(const std::string &s) {
    return (is_token(s) || is_quoted_string(s));
}

bool is_token_element(const char &c) {
    return (!(is_ctl(c) || is_tspecials(c)));
}

bool is_token(const std::string &s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_token_element(s.at(i))) {
            return false;
        }
    }
    return true;
}

bool is_tspecials(const char &c) {
    return (
        c == '(' || c == ')' || c == '<' || c == '>' || c == '@' ||
        c == ',' || c == ';' || c == ':' || c == '\\' || c == '"' ||
        c == '/' || c == '[' || c == ']' || c == '?' || c == '=' ||
        c == '{' || c == '}' || is_sp(c) || is_ht(c)
    );
}

bool is_comment(const std::string &s) {
    if (s.length() < 2) {
        return false;
    }
    if (!(s.at(0) == '(' && s.at(s.length() - 1) == ')')) {
        return false;
    }
    if (s.length() == 2) {
        return true;
    }
    const std::string &str = s.substr(1, s.length() - 2);
    return (is_ctext(str) || is_comment(str));
}

bool is_ctext(const std::string &s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (s.at(i) == '(' || s.at(i) == ')' || (is_ctl(s.at(i)) && s.at(i) != HTAB)) {
            return false;
        }
    }
    return true;
}

bool is_quoted_string(const std::string &s) {
    if (s.length() < 2) {
        return false;
    }
    if (s.length() == 2) {
        return true;
    }
    return (is_qdtext(s.substr(1, s.length() - 2)));
}

bool is_qdtext(const std::string &s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (is_dquote(s.at(i)) || is_ctl(s.at(i))) {
            return false;
        }
    }
    return true;
}

// bool is_uri(const std::string &s);            // = ( absoluteURI | relativeURI ) [ "#" fragment ]

bool is_absolute_uri(const std::string &s) {   // = scheme ":" *( uchar | reserved )
    const std::string::size_type colon_index = s.find(':', 0);
    if (colon_index == std::string::npos) {
        return false;
    }
    if (!is_scheme(s.substr(0, colon_index))) {
        return false;
    }
    for (size_t i = colon_index; i < s.length(); i++) {
        if (!(is_uchar(s.substr(i, 3))) || is_reserved(s.at(i))) {
            return false;
        }
    }
    return true;
}

bool is_relative_uri(const std::string &s) {    // = net_path | abs_path | rel_path
    return (is_net_path(s) || is_abs_path(s) || is_rel_path(s));
}

bool is_net_path(const std::string &s) {       // = "//" net_loc [ abs_path ]
    const std::string::size_type slash_index = s.find('/');
    if (slash_index == std::string::npos) {
        return (is_net_loc(s));
    } else {
        return (is_net_loc(s.substr(0, slash_index - 1)) && is_abs_path(s.substr(slash_index)));
    }
}

bool is_abs_path(const std::string &s) {       // = "/" rel_path
    if (s.at(0) != '/') {
        return false;
    }
    return (is_rel_path(s.substr(1)));
}

bool is_rel_path(const std::string &s) {       // = [ path ] [ ";" params ] [ "?" query ]
    std::string::size_type semi_colon_index = s.find(';', 0);
    std::string::size_type question_index = s.find('?', 0);

    if (question_index < semi_colon_index) {
        semi_colon_index = std::string::npos;
    }
    const std::string path = s.substr(0, (semi_colon_index == std::string::npos ? question_index : semi_colon_index));
    if (path != "" && !is_path(path)) {
        return false;
    }
    if (semi_colon_index != std::string::npos) {
        const std::string params = s.substr(semi_colon_index + 1, question_index - semi_colon_index - 1);
        if (semi_colon_index != std::string::npos && !is_params(params)) {
            return false;
        }
    }
    if (question_index != std::string::npos) {
        const std::string query = s.substr(question_index + 1);
        if (question_index != std::string::npos && !is_query(query)) {
            return false;
        }
    }
    return true;
}

bool is_path(const std::string &s) {           // = fsegment *( "/" segment )
    std::string::size_type slash_index = s.find('/', 0);
    if (!is_fsegment(s.substr(0, slash_index))) {
        return false;
    }
    while (slash_index != std::string::npos) {
        const std::string::size_type before_slash_index = slash_index + 1;
        slash_index = s.find('/', before_slash_index);
        if (!is_segment(s.substr(before_slash_index, slash_index - before_slash_index))) {
            return false;
        }
    }
    return true;
}

bool is_fsegment(const std::string &s) {       // = 1*pchar
    if (s.length() < 1) {
        return false;
    }
    return is_segment(s);
}

bool is_segment(const std::string &s) {        // = *pchar
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_pchar(s.substr(i, 3))) {
            return false;
        }
    }
    return true;
}

bool is_params(const std::string &s) {         // = param *( ";" param )
    std::string::size_type semi_colon_index = s.find(';', 0);
    if (!is_param(s.substr(0, semi_colon_index))) {
        return false;
    }
    while (semi_colon_index != std::string::npos) {
        const std::string::size_type next_word_front = semi_colon_index + 1;
        semi_colon_index = s.find(';', next_word_front);
        if (!is_param(s.substr(next_word_front, semi_colon_index - next_word_front))) {
            return false;
        }
    }
    return true;
}

bool is_param(const std::string &s) {          // = *( pchar | "/" )
    for (size_t i = 0; i < s.length(); i++) {
        if (!(is_pchar(s.substr(i, 3)) || s.at(i) == '/')) {
            return false;
        }
    }
    return true;
}

bool is_scheme(const std::string &s) {         // = 1*( ALPHA | DIGIT | "+" | "-" | "." )
    for (size_t i = 0; i < s.length(); i++) {
        if (!(is_alpha(s.at(i)) || is_digit(s.at(i)) || s.at(i) == '+' || s.at(i) == '-' || s.at(i) == '.')) {
            return false;
        }
    }
    return true;
}

bool is_net_loc(const std::string &s) {        // = *( pchar | ";" | "?" )
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_pchar(s.substr(i, 3)) && s.at(i) != ';' && s.at(i) != '?') {
            return false;
        }
    }
    return true;
}

bool is_query(const std::string &s) {          // = *( uchar | reserved )
    for (size_t i = 0; i < s.length(); i++) {
        if (!(is_uchar(s.substr(i, 3)) || is_reserved(s.at(i)))) {
            return false;
        }
    }
    return true;
}

// bool is_fragment(const std::string &s);       // = *( uchar | reserved )
bool is_pchar(const std::string &s) {          // = uchar | ":" | "@" | "&" | "=" | "+"
    if (!(is_uchar(s) || s.at(0) == ':' || s.at(0) == '@' || s.at(0) == '&' || s.at(0) == '=' || s.at(0) == '+')) {
        return false;
    }
    return true;
}

bool is_uchar(const std::string &s) {          // = unreserved | escape
    bool result = is_unreserved(s.at(0));
    try {
        result |= is_escape(s);
    } catch (std::out_of_range &e) {
        // is_escape で　 out_of_range を履く可能性はあるが、その場合は is_unreserved の結果が返されるべきなので握り潰している //
        (void)(e);
    }
    return result;
}

bool is_unreserved(const char &c) {     // = ALPHA | DIGIT | safe | extra | national
    return (is_alpha(c) || is_digit(c) || is_safe(c) || is_extra(c) || is_national(c));
}

bool is_escape(const std::string &s) {         // = "%" HEX HEX
    if (s.length() != 3) {
        return false;
    }
    return (s.at(0) == '%' && is_hex(s.at(1) && is_hex(s.at(2))));
}


bool is_reserved(const char &c) {       // = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+"
    return (c == ';' || c == '/' || c == '?' || c == ':' || c == '@' || c == '&' || c == '=' || c == '+');
}

bool is_extra(const char &c) {          // = "!" | "*" | "'" | "(" | ")" | ","
    return (c == '!' || c == '*' || c == '\'' || c == '(' || c == ')' || c == ',');
}

bool is_safe(const char &c) {           // = "$" | "-" | "_" | "."
    return (c == '$' || c == '-' || c == '_' || c == '.');
}

bool is_unsafe(const char &c) {         // = CTL | SP | <"> | "#" | "%" | "<" | ">"
    return (!(is_ctl(c) || is_sp(c) || c == '"' || c == '#' || c == '%' || c == '<' || c == '>'));
}

bool is_national(const char &c) {       // <any OCTET excluding ALPHA, DIGIT, reserved, extra, safe, and unsafe>
    return (!(!is_alpha(c) || is_digit(c) || is_reserved(c) || is_extra(c) || is_safe(c) || is_unsafe(c)));
}

bool is_http_version(const std::string &s) {   // = "HTTP" "/" 1*DIGIT "." 1*DIGIT
    try {
        const std::string http = "HTTP";
        size_t s_index = 0;
        for (size_t i = 0; i < http.length(); i++) {
            if (s.at(s_index) != http.at(i)) {
                return false;
            }
            s_index++;
        }
        if (s.at(s_index) != '/' || !is_digit(s.at(s_index + 1))) {
            return false;
        }
        s_index++;
        while (is_digit(s.at(s_index))) {
            s_index++;
        }
        if (s.at(s_index) != '.' || !is_digit(s.at(s_index + 1))) {
            return false;
        }
        s_index++;
        while (s_index < s.length() && is_digit(s.at(s_index))) {
            s_index++;
        }
        if (s_index != s.length()) {
            return false;
        }
    } catch (std::out_of_range const &) {
        std::cerr << "catch " << std::endl;
        return false;
    }
    return true;
}

bool is_valid_http_header_element(const std::string &str) {
    size_t i = 0;
    while (i < str.length() && is_filed_vchar_element(str.at(i))) {
        i++;
    }
    if (i == 0) {
        return false;
    } else if (i == str.length()) {
        return true;
    }
    while (i < str.length() && is_ows_elment(str.at(i))) {
        i++;
    }
    if (str.length() < i && str.at(i) != ',') {
        return false;
    }
    while (i < str.length() && is_ows_elment(str.at(i))) {
        i++;
    }
    if (str.length() < i && !is_filed_vchar_element(str.at(i))) {
        return false;
    }
    if (i == str.length()) {
        return false;
    }
    return is_valid_http_header_element(str.substr(i));
}

// https://www.rfc-editor.org/rfc/rfc9110.html#section-5.5-3
// これって HTABも含むの？？ //
bool is_valid_http_header(const std::string &str) {
    try {
        const std::string removed_crlf = str.substr(0, str.length() - strlen(CRLF));
        const std::string field_name = get_first_token(removed_crlf);
        if (removed_crlf.at(field_name.length()) != ':' && !is_crlf(str.substr(str.length() - strlen(CRLF)))) {
            return false;
        }
        std::string filed_value = trim_string(removed_crlf.substr(field_name.length() + 1), " ");
        if (!is_valid_http_header_element(filed_value)) {
            return false;
        }
    } catch (std::out_of_range &e) {
        return false;
    }
    return true;
}

bool is_http_date(const std::string &s) {         // HTTP-date      = rfc1123-date | rfc850-date | asctime-date
    return (is_rfc1123_data(s) || is_rfc850_data(s) || is_asctime_date(s));
}

bool is_rfc1123_data(const std::string &s) {      // rfc1123-date   = wkday "," SP date1 SP time SP "GMT"
    try {
        const std::vector<std::string> splited_s = split(s, " ");
        std::string wkday = splited_s.at(0);
        wkday.erase(wkday.end() - 2, wkday.end());
        std::string date = splited_s.at(1) + splited_s.at(2) + splited_s.at(3);
        date.erase(date.end() - 1, date.end());
        std::string time = splited_s.at(4);
        time.erase(time.end() - 1, time.end());
        std::string gmt = splited_s.at(5);
        return (
            is_wkday(wkday) &&
            splited_s.at(0).at(wkday.length()) == ',' &&
            is_date1(date) &&
            is_time(time) &&
            gmt == "GMT"
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}

bool is_rfc850_data(const std::string &s) {       // rfc850-date    = weekday "," SP date2 SP time SP "GMT"
    try {
        const std::string::size_type coron_index = s.find(',');
        const std::string::size_type first_sp_index = s.find(' ');
        const std::string::size_type second_sp_index = s.find(' ', first_sp_index + 1);
        const std::string::size_type therd_sp_index = s.find(' ', second_sp_index + 1);
        return (
            is_weekday(s.substr(0, coron_index)) &&
            coron_index + 1 == first_sp_index &&
            is_date2(s.substr(first_sp_index + 1, second_sp_index - first_sp_index - 1 )) &&
            is_time(s.substr(second_sp_index + 1, therd_sp_index - second_sp_index - 1)) &&
            s.substr(therd_sp_index + 1) == "GMT"
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}

// "Mon Jun  2 08:12:31 2222";
bool is_asctime_date(const std::string &s) {      // asctime-date   = wkday SP date3 SP time SP 4DIGIT
    try {
        const std::vector<std::string> splited_s = split(s, " ");
        std::string wkday = splited_s.at(0);
        wkday.erase(wkday.end() - 1, wkday.end());
        std::string date = splited_s.at(1) + splited_s.at(2);
        std::string time;
        std::string year;
        if (splited_s.at(2) == " ") {
            date += splited_s.at(3);
            time = splited_s.at(4);
            year = splited_s.at(5);
        } else {
            time = splited_s.at(3);
            year = splited_s.at(4);
        }
        date.erase(date.end() - 1, date.end());
        time.erase(time.end() - 1, time.end());
        return (
            is_wkday(wkday) &&
            is_date3(date) &&
            is_time(time) &&
            is_4digit(year)
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}


bool is_date1(const std::string &s) {             // date1          = 2DIGIT SP month SP 4DIGIT ; day month year (e.g., 02 Jun 1982)
    try {
        const std::string::size_type first_sp_index = s.find(' ');
        const std::string::size_type second_sp_index = s.find(' ', first_sp_index + 1);
        return (
            is_2digit(s.substr(0, first_sp_index)) &&
            is_month(s.substr(first_sp_index + 1, second_sp_index - first_sp_index - 1)) &&
            is_4digit(s.substr(second_sp_index + 1))
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}

bool is_date2(const std::string &s) {             // date2          = 2DIGIT "-" month "-" 2DIGIT ; day-month-year (e.g., 02-Jun-82)
try {
        const std::string::size_type first_dash_index = s.find('-');
        const std::string::size_type second_dash_index = s.find('-', first_dash_index + 1);
        return (
            is_2digit(s.substr(0, first_dash_index)) &&
            is_month(s.substr(first_dash_index + 1, second_dash_index - first_dash_index - 1)) &&
            is_2digit(s.substr(second_dash_index + 1))
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}

bool is_date3(const std::string &s) {             // date3          = month SP ( 2DIGIT | ( SP 1DIGIT )) ; month day (e.g., Jun  2)
    try {
        const std::string::size_type first_sp_index = s.find(' ');
        const std::string day = s.substr(first_sp_index + 1);
        if (day.length() != 2) {
            return false;
        }
        if ((day.at(0) == ' ' && !is_digit(s.at(1)))) {
            return is_month(s.substr(0, first_sp_index));
        }
        return (
            is_month(s.substr(0, first_sp_index)) &&
            is_2digit(day)
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}

bool is_time(const std::string &s) {              // time           = 2DIGIT ":" 2DIGIT ":" 2DIGIT ; 00:00:00 - 23:59:59
    try {
        return (
            is_2digit(s.substr(0, 2)) && s.at(2) == ':' &&
            is_2digit(s.substr(2 + 1, 2)) && s.at(2 + 1 + 2) == ':' &&
            is_2digit(s.substr(2 + 1 + 2 + 1, 2)) &&
            s.length() == 2 + 1 + 2 + 1 + 2
        );
    } catch (std::out_of_range const &) {
        return false;
    }
}

bool is_wkday(const std::string &s) {             // wkday          = "Mon" | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun"
    const size_t size = 7;
    const std::string wkday[size] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };
    return is_target_included_list(s, wkday, size);
}

bool is_weekday(const std::string &s) {           // weekday        = "Monday" | "Tuesday" | "Wednesday" | "Thursday" | "Friday" | "Saturday" | "Sunday"
    const size_t size = 7;
    const std::string weekday[size] = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };
    return is_target_included_list(s, weekday, size);
}

bool is_month(const std::string &s) {             // month          = "Jan" | "Feb" | "Mar" | "Apr" | "May" | "Jun" | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"
    const size_t size = 12;
    const std::string month[size] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };
    return is_target_included_list(s, month, size);
}

bool is_target_included_list(const std::string target, const std::string list[], size_t size) {
    // std::cout << "target : '" << target << "'" << std::endl;
    for (size_t i = 0; i < size; i++) {
        if (target == list[i]) {
            return true;
        }
    }
    return false;
}

bool is_4digit(const std::string &s) {
    if (s.length() != 4) {
        return false;
    }
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_digit(s.at(i))) {
            return false;
        }
    }
    return true;
}

bool is_2digit(const std::string &s) {
    if (s.length() != 2) {
        return false;
    }
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_digit(s.at(i))) {
            return false;
        }
    }
    return true;
}

bool is_pragma_directive(const std::string &s) {
    if (s == "no-cache") {
        return true;
    }
    return is_extension_pragma(s);
}

// extension-pragma = token [ "=" word ]
bool is_extension_pragma(const std::string &s) {
    const std::string::size_type equal_index = s.find('=');
    if (equal_index == std::string::npos) {
        return is_token(s);
    } else {
        return (is_token(s.substr(0, equal_index)) && is_word(s.substr(equal_index + 1)));
    }
}

bool is_product(const std::string &s) {
    const std::string::size_type slash_index = s.find('/');
    if (slash_index == std::string::npos) {
        return (is_token(s));
    } else {
        return (is_token(s.substr(0, slash_index)) && is_token(s.substr(slash_index + 1)));
    }
}

// RFC 3986
bool is_ip_literal(const std::string &s) {        // IP-literal    = "[" ( IPv6address / IPvFuture  ) "]"
    try {
        if (s.at(0) != '[' || s.at(s.length() - 1) != ']') {
            return false;
        }
        std::string trimed = s.substr(1, s.length() - strlen(CRLF));
        return (is_ipv6_address(trimed) || is_ipv_future(trimed));
    } catch (...) {
        return false;
    }
}

bool is_ipv4address(const std::string &s) {       // IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
    try {
        std::vector<std::string> splited = split(s, ".");
        if (splited.size() != 4) {
            return false;
        }
        for (size_t i = 0; i < 3; i++) {
            splited.at(i).erase(splited.at(i).end() - 1);
        }
        for (size_t i = 0; i < 4; i++) {
            if (!is_dec_octed(splited.at(i))) {
                return false;
            }
        }
    } catch (...) {
        return false;
    }
    return true;
}
bool is_reg_name(const std::string &s) {          // reg-name      = *( uri-unreserved / pct-encoded / sub-delims )
    for (size_t i = 0; i < s.length(); i++) {
        const std::string three_char = s.substr(i, 3);
        if (!is_uri_unreserved(s.at(i)) && !is_pct_encoded(three_char) && !is_sub_delims(s.at(i))) {
            return false;
        }
    }
    return true;
}


//    IPv6address   =                            6( h16 ":" ) ls32
//                  /                       "::" 5( h16 ":" ) ls32
//                  / [               h16 ] "::" 4( h16 ":" ) ls32
//                  / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
//                  / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
//                  / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
//                  / [ *4( h16 ":" ) h16 ] "::"              ls32
//                  / [ *5( h16 ":" ) h16 ] "::"              h16
//                  / [ *6( h16 ":" ) h16 ] "::"
// この関数正しく動くかわからない(検証項目が多すぎてどれが正しいのかがわからない) //
bool is_ipv6_address(const std::string &s) {
    int segment_count = 0;
    bool is_dual = false;
    std::string::size_type current_segment_front = 0;
    std::string::size_type current_segment_end = 0 - 1;
    std::string segment_str;

    do {
        current_segment_front = current_segment_end + 1;
        current_segment_end = s.find(':', current_segment_front);
        if (current_segment_front == current_segment_end) {
            if (is_dual) {
                return false;
            }
            is_dual = true;
            if (current_segment_end == 0) {
                current_segment_end++;
            }
            if (s.find(':', current_segment_end + 1) == std::string::npos) {
                break;
            }
            continue;
        }
        segment_count++;
        segment_str = s.substr(current_segment_front, current_segment_end - current_segment_front);
        if (current_segment_end == std::string::npos && is_ipv4address(segment_str)) {
            segment_count += 1;
            break;
        }
        if (!is_h16(segment_str)) {
            return false;
        }
    } while (current_segment_end != std::string::npos);

    if (is_dual) {
        return (segment_count < 8);
    } else {
        return (segment_count == 8);
    }
}

// テストしてない //
bool is_ipv_future(const std::string &s) {         // IPvFuture     = "v" 1*HEXDIG "." 1*( uri-unreserved / sub-delims / ":" )
    size_t i;
    if (s.at(0) != 'v') {
        return false;
    }
    for (i = 1; i < s.length(); i++) {
        if (s.at(i) == '.') {
            break;
        }
        if (!is_hex(s.at(i))) {
            return false;
        }
    }
    i++;
    for (; i < s.length(); i++) {
        if (!is_uri_unreserved(s.at(i)) && !is_sub_delims(s.at(i)) && s.at(i) != ':') {
            return false;
        }
    }
    return true;
}

bool is_uri_unreserved(const char &c) {           // uri-unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
    return (is_alpha(c) || is_digit(c) || c == '-' || c == '.' || c == '_' || c == '~');
}

bool is_pct_encoded(const std::string &s) {       // pct-encoded   = "%" HEXDIG HEXDIG
    return is_escape(s);
}

bool is_sub_delims(const char &c) {               // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
    return (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=');
}

bool is_h16(const std::string &s) {               // h16           = 1*4HEXDIG
    if (!(1 <= s.length() && s.length() <= 4)) {
        return false;
    }
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_hex(s.at(i))) {
            return false;
        }
    }
    return true;
}

// 結果的に使ってない //
bool is_ls32(const std::string &s) {              // ls32          = ( h16 ":" h16 ) / IPv4address
    if (is_ipv4address(s)) {
        return true;
    }
    if (s.length() != 4 + 1 + 4) {
        return false;
    }
    return ((is_h16(s.substr(0, 4)) && s.at(4) == ':' && is_h16(s.substr(5, 4))));
}

bool is_dec_octed(const std::string &s) {
    int n = safe_atoi(s);
    return (0 <= n && n <= 255 && int_to_string(n) == s);
}

bool is_port(const std::string &s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (!is_digit(s.at(i))) {
            return false;
        }
    }
    return true;
}

// int main () {
    /*
    std::string d = "Mon Jun 22 08:12:31 2222";
    // std::vector<std::string> sp = split(d, " ");
    // for (size_t i = 0; i < sp.size(); i++) {
    //     std::cout << sp[i] <<std::endl;
    // }
    std::cout << is_http_date(d) << std::endl;;
    std::cout << is_rfc1123_data(d) << std::endl;;
    std::cout << is_rfc850_data(d) << std::endl;;
    std::cout << is_asctime_date(d) << std::endl;;
    // */

    /*
    std::string strs[6] = {
        "foo,bar",
        "foo ,bar,",
        "foo , ,bar,charlie",
        "",
        ",",
        ",   ,"
    };
    for (size_t i = 0; i < 6; i++) {
        std::cout << strs[i] << ' ' << is_valid_http_header_element(strs[i]) << std::endl;
    }
    // */
// }

/*
int main() {
    std::string ipv6_address[] = {
        // "2001:db8:3333:4444:5555:6666:7777:8888",
        // "2001:db8:3333:4444:CCCC:DDDD:EEEE:FFFF",
        "2001:db8::",
        "::1234:5678",
        "2001:db8::1234:5678",
        "2001:0db8:0001:0000:0000:0ab9:C0A8:0102",
        "2001:db8:1::ab9:C0A8:102",
        "2001:db8:3333:4444:5555:6666:1.2.3.4",
        "::11.22.33.44",
        "2001:db8::123.123.123.123",
        "::1234:5678:91.123.4.56",
        "::1234:5678:1.2.3.4",
        "2001:db8::1234:5678:5.6.7.8",
        "::",

        "2001:db8:3333:4444:5555:6666:0.255.256.1",
        "2001:db8:3333:4444:5555:6666:fFfF:ghi",
        "2001:db8:3333:4444:5555:6666:1:",
        "2001:db8:3333:4444:CCCC:DDDD:EEEE:FFFF:",
        "1111::3333::4444",

        "1111:2222:3333:4444:5555:6666:7777:8888",
        "1111:2222:3333:4444:5555:6666:1.2.3.4",

        "::2222:3333:4444:5555:6666:7777:8888",
        "::2222:3333:4444:5555:6666:1.2.3.4",

        "1111::3333:4444:5555:6666:7777:8888",
        "1111::3333:4444:5555:6666:1.2.3.4",
        "::3333:4444:5555:6666:7777:8888",
        "::3333:4444:5555:6666:1.2.3.4",

        "1111:2222::4444:5555:6666:7777:8888",
        "1111:2222::4444:5555:6666:1.2.3.4",
        "1111::4444:5555:6666:7777:8888",
        "1111::4444:5555:6666:1.2.3.4",
        "::3333:4444:5555:6666:7777:8888",
        "::3333:4444:5555:6666:1.2.3.4",

        "1111:2222:3333::5555:6666:7777:8888",
        "1111:2222:3333::5555:6666:1.2.3.4",
        "1111:2222::5555:6666:7777:8888",
        "1111:2222::5555:6666:1.2.3.4",
        "1111::5555:6666:7777:8888",
        "1111::5555:6666:1.2.3.4",
        "::5555:6666:7777:8888",
        "::5555:6666:1.2.3.4",

        "1111:2222:3333:4444::6666:7777:8888",
        "1111:2222:3333:4444::6666:1.2.3.4",
        "1111:2222:3333::6666:7777:8888",
        "1111:2222:3333::6666:1.2.3.4",
        "1111:2222::6666:7777:8888",
        "1111:2222::6666:1.2.3.4",
        "1111::6666:7777:8888",
        "1111::6666:1.2.3.4",
        "::6666:7777:8888",
        "::6666:1.2.3.4",

        "1111:2222:3333:4444:5555::7777:8888",
        "1111:2222:3333:4444:5555::1.2.3.4",
        "1111:2222:3333:4444::7777:8888",
        "1111:2222:3333:4444::1.2.3.4",
        "1111:2222:3333::7777:8888",
        "1111:2222:3333::1.2.3.4",
        "1111:2222::7777:8888",
        "1111:2222::1.2.3.4",
        "1111::7777:8888",
        "1111::1.2.3.4",
        "::7777:8888",
        "::1.2.3.4",


        "1111:2222:3333:4444:5555:6666::8888",
        "1111:2222:3333:4444:5555:6666::1.2.3.4", // forbidden
        "1111:2222:3333:4444:5555::8888",
        "1111:2222:3333:4444::8888",
        "1111:2222:3333::8888",
        "1111:2222::8888",
        "1111::8888",
        "::8888",



        "1111:2222:3333:4444:5555:6666:7777::",
        "1111:2222:3333:4444:5555:6666::",
        "1111:2222:3333:4444:5555::",
        "1111:2222::",
        "1111::",
        "::",


        "END_OF_ARRAY"
    };
    for (size_t i = 0; ; i++) {
        if (ipv6_address[i] == "END_OF_ARRAY") {
            break;
        }
        std::cout << i << " : " << is_ipv6_address(ipv6_address[i]) << ' ' << ipv6_address[i] << std::endl;
    }
}
// */
