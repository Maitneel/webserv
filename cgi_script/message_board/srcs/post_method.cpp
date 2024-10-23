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

std::string get_formated_date() {
    struct tm newtime;
    time_t ltime;
    char buf[50];

    ltime = time(&ltime);
    localtime_r(&ltime, &newtime);
    std::string now_string(asctime_r(&newtime, buf));
    now_string.erase(
        remove(now_string.begin(), now_string.end(), '\n'),
        now_string.end());
    return now_string;
}

std::string create_message_div(const SimpleDB &message_db, const std::string &id) {
    std::string html;
    std::string message = message_db.noexcept_get(DB_MESSAGE_ID_PREFIX + id);
    std::string time_stamp = message_db.noexcept_get(DB_TIME_STAMP_ID_PREFIX + id);
    std::string image_content_type = message_db.noexcept_get(DB_IMAGE_CONTENT_TYPE_PREFIX + id);
    std::string image_path = IMAGE_URL_PREFIX + id;

    if (time_stamp == "") {
        time_stamp = "unknown post time";
    }

    html += "    <div class=\"message-div\" id=\"message-div-";
    html += id;
    html += "\">\n";
    html += "        <span class=\"time-stamp\" id=\"time-stamp-";
    html += id;
    html += "\">";
    html += time_stamp;
    html += "</span>\n";
    html += "        <span class=\"poster-id\" id=\"poster-id-";
    html += id;
    html += "\">skdjh</span>\n";
    html += "        <br>\n";
    if (image_content_type != "") {
        html += "        <img src=\"" + image_path + "\">\n";
        html += "        <br>";
    }
    html += "        <span class=\"message\" id=\"message-";
    html += id;
    html += "\">";
    html += message;
    html += "</span>";
    html += "    </div>\n";
    html += "    <hr>\n";

    return html;
}

std::string get_formated_id(const int &n) {
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << n;
    return (std::string(ss.str()));
}

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

void create_index_html(SimpleDB &message_db) {
    std::string html;
    html += create_template_front();
    int message_count = stoi(message_db.get(DB_MESSAGE_COUNT_ID));
    for (int i = 0; i < message_count; i++) {
        std::string id(get_formated_id(i));
        html += create_message_div(message_db, id);
    }
    html += create_template_end();

    std::ofstream ofs(INDEX_HTML_PATH);
    ofs << html << std::endl;
}

#include <fstream>

void post_method() {
    SimpleDB message_db(MESSAGE_DB_PATH);
    FormDataBody body;

    update_message_db(&message_db, body);
    create_index_html(message_db);
    get_method();
}