#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

#include "simple_db.hpp"
#include "proccessing.hpp"
#include "defines.hpp"
#include "gen_html.hpp"

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
    message_db->add((DB_TIME_STAMP_ID_PREFIX + id), get_formated_date());


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

void post_method() {
    SimpleDB message_db(MESSAGE_DB_PATH);
    update_message_db(&message_db);
    create_index_html(message_db);
    get_method();
}