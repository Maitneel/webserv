#include <string>

// 必要であるようならする(本来はやるべき） //

namespace rfc3875 {
// https://datatracker.ietf.org/doc/html/rfc3875#section-2.2

// alpha         = lowalpha | hialpha
// lowalpha      = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" |
//                 "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" |
//                 "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" |
//                 "y" | "z"
// hialpha       = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" |
//                 "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" |
//                 "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" |
//                 "Y" | "Z"

//                     digit         = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
//                 "8" | "9"
// alphanum      = alpha | digit
// OCTET         = <any 8-bit byte>
// CHAR          = alpha | digit | separator | "!" | "#" | "$" |
//                 "%" | "&" | "'" | "*" | "+" | "-" | "." | "`" |
//                 "^" | "_" | "{" | "|" | "}" | "~" | CTL
// CTL           = <any control character>
// SP            = <space character>
// HT            = <horizontal tab character>
// NL            = <newline>
// LWSP          = SP | HT | NL
// separator     = "(" | ")" | "<" | ">" | "@" | "," | ";" | ":" |
//                 "\" | <"> | "/" | "[" | "]" | "?" | "=" | "{" |
//                 "}" | SP | HT
// token         = 1*<any CHAR except CTLs or separators>
// quoted-string = <"> *qdtext <">
// qdtext        = <any CHAR except <"> and CTLs but including LWSP>
// TEXT          = <any printable character>


// bool is_text_element(const char &c);
// bool is_char(const char &c);
// bool is_token_element(const char &c);

bool is_meta_valiable_value(const std::string &s) {
    (void)(s);
    return true;
}

// meta-variable-value = "" | 1*<TEXT, CHAR or tokens of value>

}  // namespace rfc3875
