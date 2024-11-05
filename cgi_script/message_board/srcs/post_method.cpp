#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

#include "simple_db.hpp"
#include "proccessing.hpp"
#include "defines.hpp"
#include "gen_html.hpp"
#include "form_data.hpp"
#include "formated_string.hpp"
#include "cookie.hpp"

void update_text_message (SimpleDB *message_db, const FormDataBody &body, const std::string &message_id, const std::string &user_id) {
    std::string message = body.get_body("message");
    message_db->add((DB_MESSAGE_ID_PREFIX + message_id), message);
    message_db->add((DB_TIME_STAMP_ID_PREFIX + message_id), get_formated_date());
    message_db->add((DB_SENDER_PREFIX + message_id), user_id);
}

void update_image_message (SimpleDB *message_db, const FormDataBody &body, const std::string &id) {
    const std::string &attachment = body.get_body("attachment");
    if (attachment == "undefined")  {
        return;
    }
    const std::string file_path = RESOURCE_IMAGE_PREFIX + id;
    std::ofstream ofs(file_path);
    if (!ofs) {
        return;
    } 
    std::string content_type;
    try {
        content_type = body.parameters_.at("Content-Type").parameter_.begin()->first;
    } catch (...) {
        content_type = "application/octet-stream";
    }
    ofs << attachment;
    ofs.close();
    message_db->add(DB_IMAGE_CONTENT_TYPE_PREFIX + id, content_type);
    message_db->add(DB_IMAGE_PATH_PREFIX + id, file_path);
}

void update_message_db(SimpleDB *message_db, const FormDataBody &body, const std::string &user_id) {
    int message_count;
    if (message_db->is_include_key(DB_MESSAGE_COUNT_ID)) {
        message_count = std::stoi(message_db->get(DB_MESSAGE_COUNT_ID));
    } else {
        message_count = 0;
    }
    message_count++;
    message_db->update(DB_MESSAGE_COUNT_ID, std::to_string(message_count));
    std::string message_id = get_formated_id(message_count - 1);
    update_text_message(message_db, body, message_id, user_id);
    update_image_message(message_db, body, message_id);
}

#include <fstream>

void post_method(const std::multimap<std::string, std::string> &cookie) {
    SimpleDB message_db(MESSAGE_DB_PATH);
    SimpleDB auth_db(AUTH_DB_PATH);
    FormDataBody body;
    std::string cookie_header;
    std::string user_id;

    if (is_must_update(auth_db, cookie)) {
        user_id = gen_user_id(auth_db);
        std::string auth_str = gen_auth_str();
        register_auth_info_to_db(&auth_db, user_id, auth_str);
        cookie_header = get_auth_cookie_header(user_id, auth_str);
    } else {
        user_id = cookie.find(COOKIE_USER_ID_KEY)->second;
    }

    update_message_db(&message_db, body, user_id);
    create_index_html(message_db);
    std::cout << "Content-Type:text/html" << "\n";
    std::cout << cookie_header;
    std::cout << "\n\n";
}