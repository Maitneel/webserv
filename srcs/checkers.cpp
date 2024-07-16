#include "keywords.hpp"
#include <iostream>

bool is_crlf(std::string s) {
	if (s.length() != 2) {
		return false;
	}
	return (is_cr(s.at(0)) && is_lf(s.at(1)));
}

bool is_lws(std::string s) {
	if (s.length() < 3) {
		return false;
	}
	if (!is_crlf(s.substr(0,2))) {
		return false;
	}
	for (size_t i = 2; i < s.length(); i++) {
		if (!(is_sp(s.at(i)) || is_ht(s.at(i)))) {
			return false;
		}
	}
	return true;
}

bool is_text(std::string s) {
	for (size_t i = 0; i < s.length(); i++) {
		if (is_ctl(s.at(i))) {
			return false;
		}
	}
	return true;
}

bool is_hex(char c) {
	return (
		('A' <= c && c <= 'F') ||
		('a' <= c && c <= 'f') ||
		is_digit(c)
	);
	
}

bool is_word(std::string s) {
	return (is_token(s) || is_quoted_string(s));
}

bool is_token_element(char c) {
	return (!(is_ctl(c) || is_tspecials(c)));
}

bool is_token(std::string s) {
	for (size_t i = 0; i < s.length(); i++) {
		if (!is_token_element(s.at(i))) {
			return false;
		}
	}
	return true;
}

bool is_tspecials(char c) {
	return (
		c == '(' || c == ')' || c == '<' || c == '>' || c == '@' ||
		c == ',' || c == ';' || c == ':' || c == '\\' || c == '"' ||
		c == '/' || c == '[' || c == ']' || c == '?' || c == '=' ||
		c == '{' || c == '}' || is_sp(c) || is_ht(c)
	);
}

bool is_comment(std::string s) {
	if (s.length() < 2) {
		return false;
	}
	if (!(s.at(0) == '(' && s.at(s.length() - 1) == ')')) {
		return false;
	}
	if (s.length() == 2) {
		return true;
	}
	std::string str = s.substr(1, s.length() - 2);
	return (is_ctext(str) || is_comment(str));
}

bool is_ctext(std::string s) {
	for (size_t i = 0; i < s.length(); i++) {
		if (s.at(i) == '(' || s.at(i) == ')') {
			return false;
		}
	}
	return true;
}

bool is_quoted_string(std::string s) {
	if (s.length() < 2) {
		return false;
	}
	if (s.length() == 2) {
		return true;
	}
	return (is_qdtext(s.substr(1, s.length() - 2)));
}

bool is_qdtext(std::string s) {
	for (size_t i = 0; i < s.length(); i++) {
		if (is_dquote(s.at(i)) || is_ctl(s.at(i))) {
			return false;
		}
	}
	return true;
}

// bool is_uri(std::string s);            // = ( absoluteURI | relativeURI ) [ "#" fragment ]

bool is_absolute_uri(std::string s) {   // = scheme ":" *( uchar | reserved )
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
// bool is_relativeURI(std::string s);    // = net_path | abs_path | rel_path
// bool is_net_path(std::string s);       // = "//" net_loc [ abs_path ]

bool is_abs_path(std::string s) {       // = "/" rel_path
	if (s.at(0) != '/') {
		return false;
	}
	return (is_rel_path(s.substr(1)));
}

bool is_rel_path(std::string s) {       // = [ path ] [ ";" params ] [ "?" query ]
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

bool is_path(std::string s) {           // = fsegment *( "/" segment )
	std::string::size_type slash_index = s.find('/', 0);
	if (!is_fsegment(s.substr(0, slash_index))) {
		return false;
	}
	while (slash_index != std::string::npos) {
		const std::string::size_type before_slash_index = slash_index;
		slash_index = s.find('/', before_slash_index);
		if (!is_segment(s.substr(before_slash_index, slash_index - before_slash_index))) {
			return false;
		}	
	}
	return true;
}

bool is_fsegment(std::string s) {       // = 1*pchar
	if (s.length() < 1) {
		return false;
	}
	return is_segment(s);
}

bool is_segment(std::string s) {        // = *pchar
	for (size_t i = 0; i < s.length(); i++) {
		if (!is_pchar(s.substr(i, 3))) {
			return false;
		}
	}
	return true;
}

bool is_params(std::string s) {         // = param *( ";" param )
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

bool is_param(std::string s) {          // = *( pchar | "/" )
	for (size_t i = 0; i < s.length(); i++) {
		if (!(is_pchar(s.substr(i, 3)) || s.at(i) == '/')) {
			return false;
		}
	}
	return true;
}

bool is_scheme(std::string s) {         // = 1*( ALPHA | DIGIT | "+" | "-" | "." )
	for (size_t i = 0; i < s.length(); i++) {
		if (!(is_alpha(s.at(i)) || is_digit(s.at(i)) || s.at(i) == '+' || s.at(i) == '-' || s.at(i) == '.')) {
			return false;
		}
	}
	return true;
}

// bool is_net_loc(std::string s);        // = *( pchar | ";" | "?" )
bool is_query(std::string s) {          // = *( uchar | reserved )
	for (size_t i = 0; i < s.length(); i++) {
		if (!(is_uchar(s.substr(i, 3)) || is_reserved(s.at(i)))) {
			return false;
		}
	}
	return true;
}

// bool is_fragment(std::string s);       // = *( uchar | reserved )
bool is_pchar(std::string s) {          // = uchar | ":" | "@" | "&" | "=" | "+"
	if (!(is_uchar(s) || s.at(0) == ':' || s.at(0) == '@' || s.at(0) == '&' || s.at(0) == '=' || s.at(0) == '+')) {
		return false;
	}
	return true;
}

bool is_uchar(std::string s) {          // = unreserved | escape
	bool result = is_unreserved(s.at(0));
	try {
		result |= is_escape(s);
	} catch (std::out_of_range &e) {

	}
	return result;
}

bool is_unreserved(char c) {     // = ALPHA | DIGIT | safe | extra | national
	return (is_alpha(c) || is_digit(c) || is_safe(c));
}

bool is_escape(std::string s) {         // = "%" HEX HEX
	if (s.length() != 3) {
		return false;
	}
	return (s.at(0) == '%' && is_hex(s.at(1) && is_hex(s.at(2))));
}


bool is_reserved(char c) {       // = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+"
	return (c == ';' || c == '/' || c == '?' || c == ':' || c == '@' || c == '&' || c == '=' || c == '+');
}

bool is_extra(char c) {          // = "!" | "*" | "'" | "(" | ")" | ","
	return (c == '!' || c == '*' || c == '\'' || c == '(' || c == ')' || c == ',');
}

bool is_safe(char c) {           // = "$" | "-" | "_" | "."
	return (c == '$' || c == '-' || c == '_' || c == '.');
}

bool is_unsafe(char c) {         // = CTL | SP | <"> | "#" | "%" | "<" | ">"
	return (!(is_ctl(c) || is_sp(c) || c == '"' || c == '#' || c == '%' || c == '<' || c == '>'));
}

bool is_national(char c) {       // <any OCTET excluding ALPHA, DIGIT, reserved, extra, safe, and unsafe>
	return (!(!is_alpha(c) || is_digit(c) || is_reserved(c) || is_extra(c) || is_safe(c) || is_unsafe(c)));
}

bool is_http_version(std::string s) {   // = "HTTP" "/" 1*DIGIT "." 1*DIGIT
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
	} catch (std::out_of_range) {
		std::cerr << "catch " << std::endl;
		return false;
	}
	return true;
}