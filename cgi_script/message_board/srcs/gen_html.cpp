#include <string>
#include <fstream>
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
    html += "        <form action=\"\" method=\"post\" enctype=\"multipart/form-data\">\n";
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

void init_index_html() {
    std::ofstream ofs(INDEX_HTML_PATH);
    ofs << create_template_front();
    ofs << create_template_end();
    ofs.close();
}