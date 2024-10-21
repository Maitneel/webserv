#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

#include "simple_db.hpp"
#include "proccessing.hpp"
#include "defines.hpp"

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

std::string create_template_end() {
    std::string html;


    html += "    <div class=\"post-form-section\" id=\"post-form-section\">\n";
    html += "        <form action=\"\" method=\"post\">\n";
    html += "            <label for=\"message-input\">message</label>\n";
    html += "            <input type=\"text\" name=\"message\" id=\"message-input\">\n";
    html += "            <input type=\"file\" name=\"attachment\" id=\"attachment-input\">\n";
    html += "            <br>\n";
    html += "            <input type=\"submit\" value=\"post\">\n";
    html += "        </form>\n";
    html += "    </div>\n";

    html += "</body>\n";
    html += "</html>\n";

    return html;
}

std::string create_message_div(const std::string &message, const std::string &id) {
    std::string html;

    html += "    <div class=\"message-div\" id=\"message-div-";
    html += id;
    html += "\">\n";
    html += "        <span class=\"time-stamp\" id=\"time-stamp-";
    html += id;
    html += "\">1234</span>\n";
    html += "        <span class=\"poster-id\" id=\"poster-id-";
    html += id;
    html += "\">skdjh</span>\n";
    html += "        <br>\n";
    html += "        <span class=\"message\" id=\"message-";
    html += id;
    html += "\">";
    html += message;
    html += "</span>";
    html += "        <img src=\"\">\n";
    html += "    </div>\n";
    html += "    <hr>\n";

    return html;
}

std::string get_formated_id(const int &n) {
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << n;
    return (std::string(ss.str()));
}

std::string read_message() {
    std::string message, s;
    while (std::getline(std::cin, s)) {
        message += s;
    }
    return message;
}

void update_message_db(SimpleDB *message_db) {
    int message_count;
    if (message_db->is_include_key(DB_MESSAGE_COUNT_ID)) {
        message_count = std::stoi(message_db->get(DB_MESSAGE_COUNT_ID));
    } else {
        message_count = 0;
    }
    message_count++;
    message_db->update(DB_MESSAGE_COUNT_ID, std::to_string(message_count));
    std::string id = get_formated_id(message_count - 1);
    std::string message = read_message();
    message_db->add((DB_MESSAGE_ID_PREFIX + id), message);


}

void create_index_html(SimpleDB &message_db) {
    std::string html;
    html += create_template_front();
    int message_count = stoi(message_db.get(DB_MESSAGE_COUNT_ID));
    for (int i = 0; i < message_count; i++) {
        std::string id(get_formated_id(i));
        html += create_message_div(message_db.get(DB_MESSAGE_ID_PREFIX + id), id);
    }
    html += create_template_end();

    std::ofstream ofs(INDEX_HTML_PATH);
    ofs << html << std::endl;
}

void post_method() {
    SimpleDB message_db(PRIVATE_RESOURCE_ROOT + "/message_db");
    update_message_db(&message_db);
    create_index_html(message_db);
    get_method();
}