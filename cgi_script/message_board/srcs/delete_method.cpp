#include <iostream>
#include <string>
#include <simple_db.hpp>
#include "defines.hpp"
#include "gen_html.hpp"

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

    // TODO ちょっと考える　 //
    std::cin >> id;
    std::cerr << "deletemethod : id -------------------------------------------" <<std::endl;
    delete_db_data(&message_db, id);
    create_index_html(message_db);
}