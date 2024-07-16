#include "keywords.hpp"

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

bool is_hex(std::string s) {
	for (size_t i = 0; i < s.length(); i++) {
		if (!(
			('A' <= s.at(i) && s.at(i) <= 'F') ||
			('a' <= s.at(i) && s.at(i) <= 'f') ||
			is_digit(s.at(i))
		)) {
			return false;
		}
	}
	return true;
	
}

bool is_word(std::string s) {
	return (is_token(s) || is_quoted_string(s));
}

bool is_token(std::string s) {
	for (size_t i = 0; i < s.length(); i++) {
		if (is_ctl(s.at(i)) || is_tspecials(s.at(i))) {
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
