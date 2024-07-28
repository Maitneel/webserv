#include <iostream>
#include <vector>

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
        if (s.at(i) == '(' || s.at(i) == ')') {
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
    std::string::size_type slash_index = s.find('/');
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

bool is_valid_http_header(const std::string &str) {
    try {
        std::string removed_crlf = str.substr(0, str.length() - 2);
        std::string field_name = get_first_token(removed_crlf);
        std::string filed_value = removed_crlf.substr(field_name.length() + 1);
        if (removed_crlf.at(field_name.length()) != ':' && !is_crlf(str.substr(str.length() - 2))) {
            return false;
        }
        for (size_t i = 0; i < removed_crlf.length(); i++) {
            if (!is_text_element(removed_crlf.at(i))) {
                if (!is_lws(removed_crlf.substr(i, 3))) {
                    return false;
                }
            }
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
        std::vector<std::string> splited_s = split(s, " ");
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
        std::string::size_type coron_index = s.find(',');
        std::string::size_type first_sp_index = s.find(' ');
        std::string::size_type second_sp_index = s.find(' ', first_sp_index + 1);
        std::string::size_type therd_sp_index = s.find(' ', second_sp_index + 1);
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
        std::vector<std::string> splited_s = split(s, " ");
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
        std::string::size_type first_sp_index = s.find(' ');
        std::string::size_type second_sp_index = s.find(' ', first_sp_index + 1);
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
        std::string::size_type first_dash_index = s.find('-');
        std::string::size_type second_dash_index = s.find('-', first_dash_index + 1);
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
        std::string::size_type first_sp_index = s.find(' ');
        std::string day = s.substr(first_sp_index + 1);
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
    size_t size = 7;
    std::string wkday[size] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };
    return is_target_included_list(s, wkday, size);
}

bool is_weekday(const std::string &s) {           // weekday        = "Monday" | "Tuesday" | "Wednesday" | "Thursday" | "Friday" | "Saturday" | "Sunday"
    size_t size = 7;
    std::string weekday[size] = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };
    return is_target_included_list(s, weekday, size);
}

bool is_month(const std::string &s) {             // month          = "Jan" | "Feb" | "Mar" | "Apr" | "May" | "Jun" | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"
    size_t size = 12;
    std::string month[size] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };
    return is_target_included_list(s, month, size);
}

bool is_target_included_list(std::string target, std::string list[], size_t size) {
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
    std::string::size_type equal_index = s.find('=');
    if (equal_index == std::string::npos) {
        return is_token(s);
    } else {
        return (is_token(s.substr(0, equal_index)) && is_word(s.substr(equal_index + 1)));
    }
}

// int main () {
//     std::string d = "Mon Jun 22 08:12:31 2222";
//     // std::vector<std::string> sp = split(d, " ");
//     // for (size_t i = 0; i < sp.size(); i++) {
//     //     std::cout << sp[i] <<std::endl;
//     // }
//     std::cout << is_http_date(d) << std::endl;;
//     std::cout << is_rfc1123_data(d) << std::endl;;
//     std::cout << is_rfc850_data(d) << std::endl;;
//     std::cout << is_asctime_date(d) << std::endl;;
// }
