#include <map>
#include <utility>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "simple_db.hpp"

const std::string SimpleDB::empty_string = "";

SimpleDB::SimpleDB(const std::string &file_path) : filename_(file_path) {
    std::ifstream ifs(file_path.c_str());
    if (!ifs) {
        return;
    }
    while (!ifs.fail()) {
        std::string s, t;
        size_t quote_count = 0;
        do {
            t = "";
            if (!getline(ifs, t)) {
                if (s == "") {
                    break;
                } else {
                    return;
                }
            }
            s += t;
            quote_count += std::count(t.begin(), t.end(), '"');
        } while (quote_count < 4);
        try {
            this->data_.insert(this->parse_data_line(s));
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

SimpleDB::~SimpleDB() {
    std::ofstream ofs(this->filename_.c_str());
    if (!ofs) {
        std::cerr << this->filename_ << std::endl;
        // ユーザーの何らかの解決を促してもいいかもしれない //
        std::cerr << "fatal error: DB could not save" << std::endl;
        return ;
    }
    for (std::map<std::string, std::string>::iterator it = this->data_.begin(); it != this->data_.end(); it++) {
        ofs << this->make_data_line(*it) << "\n";
    }
    ofs << std::endl;
}

void SimpleDB::add(const std::string &key, const std::string &value) {
    if (this->data_.find(key) != this->data_.end()) {
        std::string error_message = "SimpleDB::add error: '";
        error_message += key;
        error_message += "' is already included";
        throw std::runtime_error(error_message);
    }
    this->data_.insert(std::make_pair(key, value));
}

void SimpleDB::remove(const std::string &key) {
    this->data_.erase(key);
}

void SimpleDB::update(const std::string &key, const std::string &value) {
    if (this->data_.find(key) == this->data_.end()) {
        this->data_.insert(make_pair(key, value));
    } else {
        this->data_.at(key) = value;
    }
}

const std::string &SimpleDB::get(const std::string &key) const {
    if (this->data_.find(key) == this->data_.end()) {
        std::string error_message = "SimpleDB::get error: '";
        error_message += key;
        error_message += "' is not included";
        throw std::runtime_error(error_message);
    }
    return this->data_.at(key);
}

bool SimpleDB::is_include_key(const std::string &key) const {
    return (this->data_.find(key) != this->data_.end());
}

const std::string &SimpleDB::noexcept_get(const std::string &key) const throw() {
    try {
        return this->get(key);
    } catch (...) {
        return (empty_string);
    }
}

std::vector<std::string> SimpleDB::get_include_key() {
    std::vector<std::string> key_list;
    for (std::map<std::string, std::string>::iterator it = this->data_.begin(); it != this->data_.end(); it++) {
        key_list.push_back(it->first);
    }
    return key_list;
}

// ----------------- private function -----------------------------

std::string get_first_quoted_string(const std::string &s, std::string::size_type front) {
    std::string quoted_string;
    bool is_quoted = false;
    bool is_escaped = false;
    for (std::string::size_type i = front; i < s.length(); i++) {
        if (s.at(i) == '\\' && !is_escaped) {
            is_escaped = true;
        } else if (s.at(i) == '"' && !is_escaped) {
            is_quoted ^= true;
            if (is_quoted == false) {
                break;
            }
        } else if (is_quoted) {
            quoted_string += s.at(i);
            is_escaped = false;
        }
    }
    if (is_quoted) {
        throw std::runtime_error("DB file: invalid format");
    }
    return quoted_string;
}

std::pair<std::string, std::string> SimpleDB::parse_data_line(const std::string &s) {
    std::string key, value;
    key = get_first_quoted_string(s, 0);
    value = get_first_quoted_string(s, s.find(":"));
    return make_pair(key, value);
}

std::string escape(const std::string &s) {
    std::string escaped;
    for (size_t i = 0; i < s.length(); i++) {
        if (s.at(i) == '"' || s.at(i) == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(s.at(i));
    }
    return escaped;
}

std::string SimpleDB::make_data_line(const std::pair<std::string, std::string> &src) {
    std::string data_line;
    data_line += '"';
    data_line += escape(src.first);
    data_line += '"';
    data_line += ':';
    data_line += '"';
    data_line += escape(src.second);
    data_line += '"';
    return data_line;
}