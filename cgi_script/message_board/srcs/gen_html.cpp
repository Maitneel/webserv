#include <string>
#include <fstream>
#include "defines.hpp"
#include "simple_db.hpp"
#include "formated_string.hpp"

#include <iostream>
using std::cerr;
using std::endl;

std::string create_template_front() {
    std::string html;
    html += "<!DOCTYPE html>\n";
    html += "<html lang=\"ja\">\n";
    html += "<head>\n";
    html += "    <meta charset=\"UTF-8\">\n";
    html += "    <title>message board</title>\n";
    html += "</head>\n";
    html += "<body>\n";

    return html;
}

std::string create_message_div(const SimpleDB &message_db, const std::string &id) {
    std::string html;
    std::string message = message_db.noexcept_get(DB_MESSAGE_ID_PREFIX + id);
    std::string time_stamp = message_db.noexcept_get(DB_TIME_STAMP_ID_PREFIX + id);
    std::string image_content_type = message_db.noexcept_get(DB_IMAGE_CONTENT_TYPE_PREFIX + id);
    std::string sender = message_db.noexcept_get(DB_SENDER_PREFIX + id);
    std::string image_path = IMAGE_URL_PREFIX + id;

    if (time_stamp == "") {
        time_stamp = "unknown post time";
    }
    if (sender == "") {
        sender = "unknown";
    }

    html += "    <div class=\"message-div\" id=\"message-div-" + id + "\">\n";
    html += "        <span class=\"message-id\" id=\"messge-id-" + id + "\">message: "  + id + "</span>\n";
    html += "        <span class=\"time-stamp\" id=\"time-stamp-" + id + "\">date: " + time_stamp + "</span>\n";
    html += "        <span class=\"sender-id\" id=\"sender-id-" + id + "\">" + sender + "</span>\n";
    html += "        <button class=\"delete-button\" id=\"delete-button-" + id + "\">delete</button>\n";
    html += "        <br>\n";
    if (image_content_type != "") {
        html += "        <img src=\"" + image_path + "\">\n";
        html += "        <br>";
    }
    html += "        <span class=\"message\" id=\"message-" + id + "\">" + message + "</span>\n";
    html += "    </div>\n";
    html += "    <hr>\n";

    return html;
}

std::string create_template_end() {
    std::string html;

    html += "    <div class=\"post-form-section\" id=\"post-form-section\">\n";
    // html += "        <form action=\"\" method=\"post\" enctype=\"multipart/form-data\">\n";
    html += "        <div class=\"post-section\" id=\"post-secition-id\">\n";
    html += "            <label for=\"message-input\">message</label>\n";
    html += "            <input type=\"text\" name=\"message\" id=\"message-input\">\n";
    html += "            <input type=\"file\" name=\"attachment\" id=\"attachment-input\">\n";
    html += "            <br>\n";
    // html += "            <input type=\"submit\" value=\"post\">\n";
    html += "            <button class=\"submit-button\" id=\"submit-button-id\">post</button>\n";
    html += "        </div>\n";
    // html += "        </form>\n";
    html += "    </div>\n";

    std::string s;
    std::ifstream request_js_file(JS_FILE_PATH);
    html += "    <script>\n";
    while (std::getline(request_js_file, s)) {
        html += "    " + s + "\n";
    }
    html += "    </script>\n";

    html += "</body>\n";
    html += "</html>\n";

    return html;
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

void init_index_html() {
    std::ofstream ofs(INDEX_HTML_PATH);
    ofs << create_template_front();
    ofs << create_template_end();
    ofs.close();
}