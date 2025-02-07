#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "form_data.hpp"

#define CRLF "\r\n"

using std::cerr;
using std::endl;
#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

// ---------------- utility function -------------------

#define PARAMETER_DALIMITER " :\0x09"
#define BUFFER_SIZE 1024

static std::string get_cin_buffer(int length) {
    std::string stdin_date;
    char buffer[BUFFER_SIZE];
    while (0 < length) {
        int read_ret = read(STDIN_FILENO, buffer, std::min(length, BUFFER_SIZE));
        if (read_ret < 0) {
            throw std::runtime_error("read error");
        }
        stdin_date.append(buffer, read_ret);
        length -= read_ret;
    }
    return stdin_date;
}

static std::string get_boundary() {
    const std::string content_env = getenv("CONTENT_TYPE");
    std::string::size_type boundary_value_front = content_env.find("boundary=") + strlen("boundary=");
    std::string::size_type boundary_value_end = content_env.find_first_of(PARAMETER_DALIMITER, boundary_value_front);
    return content_env.substr(boundary_value_front, boundary_value_end - boundary_value_front);
}

static std::vector<std::string> split_with_erase_delimiter(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> splited;
    std::string::size_type split_front = 0;
    std::string::size_type split_end = 0;

    do {
        split_end = str.find(delimiter, split_front);
        splited.push_back(str.substr(split_front, split_end - split_front));
        split_front = split_end + delimiter.length();
    } while (split_end != std::string::npos);
    return splited;
}

std::string trim_str(std::string *str, const std::string &remove_char) {
    std::string::size_type front = str->find_first_not_of(remove_char);
    std::string::size_type last = str->find_last_not_of(remove_char) + 1;
    if (str->length() < front) {
        *str = "";
    } else {
        *str = str->substr(front, last - front);
    }
    return *str;
}

std::string trim_front_str(std::string *str, const std::string &remove_char) {
    std::string::size_type front = str->find_first_not_of(remove_char);
    if (str->length() < front) {
        *str = "";
    } else {
        *str = str->substr(front);
    }
    return *str;
}

// ----------------- FormDataParameters ---------------

std::pair<std::string, std::string> parse_parameter(const std::string &str) {
    std::string::size_type equal_index = str.find("=");
    std::string name = str.substr(0, equal_index);
    std::string value = "";
    if (equal_index + 1 < str.length() && equal_index != std::string::npos) {
        value = str.substr(equal_index + 1);
        trim_str(&value, "\"");
    }
    return std::make_pair(name, value);
} 

FormDataParameters::FormDataParameters(const std::string &single_content) {
    std::vector<std::string> splited_by_lf = split_with_erase_delimiter(single_content, CRLF);
    for (size_t i = 0; i < splited_by_lf.size(); i++) {
        debug(i);
        std::string &line = splited_by_lf[i];
        if (line == "") {
            break;
        }
        try {
            std::string::size_type colon_index = line.find(":");
            const std::string name = line.substr(0, colon_index);
            this->parameter_.insert(make_pair(name, std::map<std::string, std::string>()));
            if (line.length() < colon_index) {
                continue;
            }
            std::string parameter_line = line.substr(colon_index);
            std::vector<std::string> splited_by_semi_colon = split_with_erase_delimiter(parameter_line, ";");
            for (size_t i = 0; i < splited_by_semi_colon.size(); i++) {
                trim_str(&splited_by_semi_colon[i], PARAMETER_DALIMITER);
                this->parameter_.at(name).insert(parse_parameter(splited_by_semi_colon[i]));
            }
            for (auto it = this->parameter_.at(name).begin(); it != this->parameter_.at(name).end(); it++) {
                cerr << "name: '" << it->first << "' '" << it->second << "'" <<  endl;
            }
        } catch (std::exception &e) {
            std::cerr << "FormParameter: " << e.what() << " line: " << line << std::endl;
            continue;
        }
    }
}

FormDataParameters::~FormDataParameters() {

}

// ---------------- FormDataBody  ---------------------

std::string get_content_entity(const std::string &str, size_t lf_count) {
    size_t body_start = 0;
    while (body_start < str.length() && lf_count) {
        if (str[body_start] == '\n') {
            lf_count--;
        }
        body_start++;
    }
    // debug(body_start);
    // debug(cr_count);
    // debug(str.substr(body_start, str.length() - body_start - 1));
    body_start += strlen(CRLF);
    return str.substr(body_start, str.length() - body_start - strlen(CRLF));
}

std::string replace_crlf_to_lf(const std::string &str) {
    std::string replaced;
    for (size_t i = 0; i < str.length() - 1; i++) {
        if (!(str.at(i) == '\r' && str.at(i + 1) == '\n')) {
            replaced += str.at(i);
        }
    }
    replaced += str.at(str.length() - 1);
    return replaced;
}

FormDataBody::FormDataBody(const int &length) : buffer_(get_cin_buffer(length)) {
    const std::string boundary = "--" + get_boundary();
    std::vector<std::string> splited_by_boundary = split_with_erase_delimiter(this->buffer_, boundary);

    for (size_t i = 0; i < splited_by_boundary.size(); i++) {
        std::string name;

        trim_front_str(&splited_by_boundary[i], CRLF);
        if (splited_by_boundary[i].substr(0, 2) == "--") {
            break;
        }
        try {
            FormDataParameters temp_parameter(splited_by_boundary[i]);
            name = temp_parameter.parameter_.at("Content-Disposition").at("name");
            this->parameters_.insert(make_pair(name, temp_parameter));
            this->body_.insert(std::make_pair(name, get_content_entity(splited_by_boundary[i], this->parameters_.at(name).parameter_.size())));
        } catch (std::exception &e) {
            std::cerr << "FormDataParameter: " << e.what() << std::endl;
            continue;
        }
    }
}

FormDataBody::~FormDataBody() {

}

const std::string &FormDataBody::get_body(const std::string &name) const {
    return this->body_.at(name);
}
