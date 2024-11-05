#include <iostream>
#include <string>
#include <simple_db.hpp>
#include "defines.hpp"
#include "gen_html.hpp"
#include "cookie.hpp"

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

void delete_method(const std::multimap<std::string, std::string> &cookie) {
    std::string id;
    SimpleDB message_db(MESSAGE_DB_PATH);
    SimpleDB auth_db(AUTH_DB_PATH);

    // TODO ちょっと考える　 //
    std::cin >> id;
    try {
        if (challenge_auth(auth_db, cookie, message_db.noexcept_get(DB_SENDER_PREFIX + id))) {
            delete_db_data(&message_db, id);
            create_index_html(message_db);
            std::cout << "Content-Type:text/html" << "\n";
            std::cout << "\n";
        } else {
            std::cerr << "hogehoe" << std::endl;
            std::cout << "Content-Type:text/html" << "\n";
            std::cout << "Status:403 Forbidden\n";
            std::cout << "\n";
        }
    } catch (std::exception &e)  {
        std::cerr << e.what() <<std::endl;
    }
}
