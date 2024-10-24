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

void update_text_message (SimpleDB *message_db, const FormDataBody &body, const std::string &id) {
    std::string message = body.get_body("message");
    message_db->add((DB_MESSAGE_ID_PREFIX + id), message);
    message_db->add((DB_TIME_STAMP_ID_PREFIX + id), get_formated_date());
}

void update_image_message (SimpleDB *message_db, const FormDataBody &body, const std::string &id) {
    const std::string &attachment = body.get_body("attachment");
    if (attachment.size() == 0)  {
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

void update_message_db(SimpleDB *message_db, const FormDataBody &body) {
    int message_count;
    if (message_db->is_include_key(DB_MESSAGE_COUNT_ID)) {
        message_count = std::stoi(message_db->get(DB_MESSAGE_COUNT_ID));
    } else {
        message_count = 0;
    }
    message_count++;
    message_db->update(DB_MESSAGE_COUNT_ID, std::to_string(message_count));
    std::string id = get_formated_id(message_count - 1);
    update_text_message(message_db, body, id);
    update_image_message(message_db, body, id);
}

#include <fstream>

void post_method() {
    SimpleDB message_db(MESSAGE_DB_PATH);
    FormDataBody body;

    update_message_db(&message_db, body);
    create_index_html(message_db);
    get_method();
}