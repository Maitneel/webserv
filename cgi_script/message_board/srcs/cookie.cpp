#include <string>

#include "simple_db.hpp"
#include "defines.hpp"
#include "string_lib.hpp"

#include <iostream>

std::string get_remove_cookie(std::string name) {
    return ("Set-Cookie: " + name + "; Max-Age=-1\n");
}

std::string gen_user_id(const SimpleDB &auth_db) {
    std::string user_id;
    do {
        user_id = gen_random_str(USER_ID_LENGTH);
    } while (auth_db.is_include_key(user_id));
    return user_id;
}

std::string gen_auth_str() {
    return gen_random_str(USER_AUTH_STRING_LENGTH);
}

void register_auth_info_to_db(SimpleDB *auth_db, std::string user_id, std::string auth_str) {
    auth_db->add(user_id, auth_str);
}

std::string get_auth_cookie_header(const std::string &user_id, const std::string &auth_str) {
    std::string cookie_header;
    cookie_header += "Set-Cookie: " + COOKIE_USER_ID_KEY + "=" + user_id + "; Max-Age=" + COOKIE_MAX_AGE + "\n"; 
    cookie_header += "Set-Cookie: " + COOKIE_USER_AUTH_KEY + "=" + auth_str + "; Max-Age=" + COOKIE_MAX_AGE + "\n"; 

    return cookie_header;
}

std::string gen_new_auth_cookie(SimpleDB *auth_db) {
    std::string user_id = gen_user_id(*auth_db);
    std::string auth_str = gen_auth_str();
    register_auth_info_to_db(auth_db, user_id, auth_str);
    return get_auth_cookie_header(user_id, auth_str);
}

bool is_must_update(const SimpleDB &auth_db, const std::multimap<std::string, std::string> &cookie) {
    if (cookie.count(COOKIE_USER_ID_KEY) == 1 && cookie.count(COOKIE_USER_AUTH_KEY) == 1) {
        if (auth_db.noexcept_get(cookie.find(COOKIE_USER_ID_KEY)->second) == cookie.find(COOKIE_USER_AUTH_KEY)->second) {
            return false;
        }
    }
    std::cerr << "must_update true;" <<std::endl;
    return true;
}

bool is_remove_prev_cookie(const SimpleDB &auth_db, const std::multimap<std::string, std::string> &cookie) {
    if ((cookie.count(COOKIE_USER_ID_KEY) == 0 && cookie.count(COOKIE_USER_AUTH_KEY) == 0) || !is_must_update(auth_db, cookie)) {
        return false;
    }
    std::cerr << "is_remove true" << std::endl;
    return true;
}

std::string get_cookie_header(const std::multimap<std::string, std::string> &cookie, SimpleDB *auth_db) {
    if (cookie.count(COOKIE_USER_ID_KEY) == 0 && cookie.count(COOKIE_USER_AUTH_KEY) == 0) {
        return gen_new_auth_cookie(auth_db);
    }
    if (!is_must_update(*auth_db, cookie)) {
        return "";
    }

    return (
        get_remove_cookie(COOKIE_USER_ID_KEY) +
        get_remove_cookie(COOKIE_USER_AUTH_KEY) +
        gen_new_auth_cookie(auth_db)
    );
}

std::multimap<std::string, std::string> parse_cookie() {
    const char *cookie_cstr = getenv("HTTP_COOKIE");
    if (cookie_cstr == NULL) {
        return std::multimap<std::string, std::string>();
    }
    const std::string cookie_str = cookie_cstr;
    std::vector<std::string> splited = escaped_quote_split(cookie_str, ";");
    std::multimap<std::string, std::string> cookie;
    for (size_t i = 0; i < splited.size(); i++) {
        std::vector<std::string> cookie_pair = escaped_quote_split(splited[i], "=");
        
        if (2 < cookie_pair.size()) {
            continue;
        }
        std::string key = cookie_pair.at(0);
        std::string value;
        if (cookie_pair.size() == 2) {
            value = cookie_pair.at(1);
        }
        key = trim(key, "= \t");
        value = trim(value, "; \t");
        cookie.insert(std::make_pair(key, value));
    }
    return cookie;
}