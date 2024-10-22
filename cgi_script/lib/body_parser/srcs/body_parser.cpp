#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "body_parser.hpp"

#define PARAMETER_DALIMITER " :\0x09"

using std::cerr;
using std::endl;
// #define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

// もう少しスマートなやり方にしたい気がする //
static std::string get_cin_buf() {
    std::stringstream ss;
    ss << std::cin.rdbuf();
    return ss.str();
}

static std::string get_boundary(const std::string &content_env) {
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
    std::string::size_type last = str->find(str->find_last_not_of(remove_char, front));
    if (str->length() < front) {
        *str = "";
    } else {
        *str = str->substr(front, last - front);
    }
    return *str;
}

static std::pair<std::string, std::string> parse_parameter(std::string line) {
    std::pair<std::string, std::string> parameter;

    try {
        trim_str(&line, " \0x09");
        std::string name = line.substr(0, line.find('='));
        std::string value("");
        if (name.length() + 1 < line.length()) {
            value = line.substr(name.length() + 1);
        }
        parameter.first = name;
        parameter.second = value;
    } catch (std::exception &e) {
        // cerr << "except: splited: " << line << ' ' << e.what() << endl;
    }
    return parameter;
}

ContentParameters::ContentParameters(const std::string &str) {
    this->key_ = str.substr(0, str.find_first_of(PARAMETER_DALIMITER));

    std::vector<std::string> splited = split_with_erase_delimiter(str.substr(str.find_first_not_of(this->key_ + PARAMETER_DALIMITER)), ";");
    for (size_t i = 0; i < splited.size(); i++) {
        this->parameter_.insert(parse_parameter(splited[i]));
    }
}

ContentElement::ContentElement(const std::string &str) {
    std::vector<std::string> splited_by_lf = split_with_erase_delimiter(str, "\n");

    for (size_t i = 0; i < splited_by_lf.size(); i++) {
        if (splited_by_lf[i] == "") {
            break;
        }
        ContentParameters parameter(splited_by_lf[i]);
        this->parameter_.insert(std::make_pair(parameter.key_, parameter));
    }

    std::string::size_type body_front = 0;
    try {
        size_t lf_count = this->parameter_.size() + 1;
        while (lf_count && body_front < str.length()) {
            if (str.at(body_front) == '\n') {
                lf_count--;
            }
            body_front++;
        }
        this->name_ = this->parameter_.at("Content-Disposition").parameter_.at("name");
        this->body_ = str.substr(body_front, str.length() - body_front - 1);
    } catch (std::exception &e) {
        // std::cerr << "except " <<  body_front << ' ' << e.what() <<endl;
    }
}

ContentElement::~ContentElement() {
}

BodyParser::BodyParser() : buffer_(get_cin_buf()) {
    std::string content_env = getenv("CONTENT_TYPE");
    std::string content_type = content_env.substr(0, content_env.find_first_of(PARAMETER_DALIMITER) - 1);
    if (content_type != "multipart/form-data") {
        throw std::runtime_error("BodyParser: unsported media type");
    }
    std::string boundary = "--" + get_boundary(content_env);
    std::vector<std::string> splited = split_with_erase_delimiter(this->buffer_, boundary);
    for (size_t i = 0; i < splited.size(); i++) {
        try {
            if (splited[i].at(0) == '\n') {
                splited[i] = splited[i].substr(1);
            }
            if (splited[i] == "--" || splited[i] == "--\n") {
                break;
            }
            ContentElement element(splited[i]);
            this->parsed_body_.insert(std::make_pair(element.parameter_.at("Content-Disposition").parameter_.at("name"), element));
        } catch (std::exception &e) {
            // cerr << "bodyparser_constructor except : '" << splited[i] << "' " << e.what() << endl;
        }
    }
    

    // 'multipart/form-data; boundary=----webkitformboundarypezxe32nbocyff8y'

}

BodyParser::~BodyParser() {
}

const std::string &BodyParser::get_body(const std::string &name) {
    if (this->parsed_body_.find(name) == this->parsed_body_.end()) {
        std::string error_message;
        error_message += "BodyParser: ";
        error_message += name;
        error_message += "dose not found";
        throw std::runtime_error(error_message);
    }
    return this->parsed_body_.at(name).body_;
}

const std::string &BodyParser::get_body() {
    if (this->parsed_body_.size() != 1) {
        std::string error_message;
        error_message += "BodyParser: element is not just one";
        throw std::runtime_error(error_message);
    }
    return (this->parsed_body_.begin()->second.body_);
}