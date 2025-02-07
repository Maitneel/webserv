#include <unistd.h>
#include <iostream>
#include <string>
#include <simple_db.hpp>
#include <cstdlib>
#include "defines.hpp"
#include "gen_html.hpp"
#include "cookie.hpp"

#define BUFFER_SIZE 1024

void delete_db_data(SimpleDB *message_db, const std::string &id) {
    if (message_db->is_include_key(DB_MESSAGE_ID_PREFIX + id)) {
        message_db->update(DB_MESSAGE_ID_PREFIX + id, "deleted");
    }
    if (message_db->is_include_key(DB_IMAGE_ID_PREFIX + id)) {
        message_db->remove(DB_IMAGE_ID_PREFIX + id);
    }
    if (message_db->is_include_key(DB_IMAGE_PATH_PREFIX + id)) {
        message_db->remove(DB_IMAGE_PATH_PREFIX + id);
    }
    if (message_db->is_include_key(DB_IMAGE_CONTENT_TYPE_PREFIX + id)) {
        message_db->remove(DB_IMAGE_CONTENT_TYPE_PREFIX + id);
    }
}


std::string read_body(int length) {
    std::string str;
    while (0 < length) {
        char buffer[BUFFER_SIZE];
        int read_ret = read(STDIN_FILENO, buffer, std::min(length, BUFFER_SIZE));
        str.append(buffer, read_ret);
        length -= read_ret;
    }
    return str;
}

void delete_method(const std::multimap<std::string, std::string> &cookie) {
    std::string id;
    SimpleDB message_db(MESSAGE_DB_PATH);
    SimpleDB auth_db(AUTH_DB_PATH);

    try {
        if (getenv("CONTENT_LENGTH") == NULL) {
            throw std::runtime_error("CONTENT_LENGH is NULL");
        }
        id = read_body(std::atoi(getenv("CONTENT_LENGTH")));
        if (challenge_auth(auth_db, cookie, message_db.noexcept_get(DB_SENDER_PREFIX + id))) {
            delete_db_data(&message_db, id);
            create_index_html(message_db);
            std::cout << "Content-Type:text/html" << "\n";
            std::cout << "\n";
            return;
        }
    } catch (std::exception &e)  {
        std::cerr << e.what() <<std::endl;
    }
    std::cerr << "hogehoe" << std::endl;
    std::cout << "Content-Type:text/html" << "\n";
    std::cout << "Status:403 Forbidden\n";
    std::cout << "\n";
}
