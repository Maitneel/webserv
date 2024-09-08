#include <string>
#include <cctype>

void to_lower(std::string *str) {
    for (size_t i = 0; i < str->length(); i++) {
        str->at(i) = tolower(str->at(i));
    }
}

void to_upper(std::string *str) {
    for (size_t i = 0; i < str->length(); i++) {
        str->at(i) = toupper(str->at(i));
    }
}
