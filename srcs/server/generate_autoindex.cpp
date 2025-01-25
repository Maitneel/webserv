#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <set>

#include "http_exception.hpp"

#define AUTOINDEX_WIDTH 50

static std::string gen_header(const std::string &req_path) {
    std::string header;
    header += "<!DOCTYPE html>";
    header += "<html>";
    header += "<head>";
    header += "    <title>Index of " + req_path + "</title>";
    header += "</head>";

    return header;
}

struct FileStat {
 public:
    std::string name;
    std::string path;
    __int64_t size;
    bool is_dir;
    FileStat(const std::string &name_arg, const std::string &path_arg, const __int64_t &size_arg, const bool &is_dir_arg) : name(name_arg), path(path_arg), size(size_arg), is_dir(is_dir_arg) {}
};

bool operator<(const FileStat &lhs, const FileStat &rhs) {
    if (lhs.is_dir != rhs.is_dir) {
        return lhs.is_dir;
    }
    return (lhs.name < rhs.name);
}

static std::string gen_html_text(const std::set<FileStat> &dir_info, const std::string &req_path) {
    std::stringstream html_string;
    html_string << "<h1>Index of " << req_path << "</h1>\n<hr>\n<pre>\n";
    html_string << "<a href=\"" << req_path << "\">" << "../" << "</a>\n";
    for (std::set<FileStat>::const_iterator it = dir_info.begin(); it != dir_info.end(); it++) {
        std::string file_name = it->name;
        if (AUTOINDEX_WIDTH < file_name.length()) {
            file_name = file_name.substr(0, AUTOINDEX_WIDTH - 3);
            file_name += "..>";
        }
        html_string << "<a href=\"" << it->path << "\">" << file_name << "</a>" << std::setw(AUTOINDEX_WIDTH - file_name.length()) << "" << ' ';
        if (it->is_dir) {
            html_string << "-" << "\n";
        } else {
            html_string << it->size << "\n";
        }
    }
    html_string << "</pre><hr>";

    return html_string.str();
}

static FileStat create_filestat(const struct dirent *file_element, const std::string &dir_name, const std::string &req_path) {
    std::string file_name = file_element->d_name;
    // TODO(maitneel): dir_nameは常に'/'で終わるべきなので本来"/"を追加する必要はない //
    std::string file_path = dir_name + "/" + file_name;
    struct stat file_info;
    if (stat(file_path.c_str(), &file_info) != 0) {
        throw MustReturnHTTPStatus(500);
    }
    if ((file_info.st_mode & S_IFDIR)) {
        file_name += '/';
        return (FileStat(file_name, req_path + file_name, file_info.st_size, true));
    } else {
        return (FileStat(file_name, req_path + file_name, file_info.st_size, false));
    }
}

static std::string gen_index(const std::string &dir_name, const std::string &req_path) {
    DIR *dir = opendir(dir_name.c_str());
    if (dir == NULL) {
        throw MustReturnHTTPStatus(500);
    }

    std::set<FileStat> dir_info;
    struct dirent *file_element;
    while ((file_element = readdir(dir)) != NULL) {
        if (file_element->d_name[0] != '.') {
            dir_info.insert(create_filestat(file_element, dir_name, req_path));
        }
    }
    return gen_html_text(dir_info, req_path);
}

static std::string gen_body(const std::string &dir_name, const std::string &req_path) {
    std::string body;
    body += "<body>";
    body += gen_index(dir_name, req_path);
    body += "</body>";
    return body;
}


std::string generate_autoindex_file(const std::string &dir_name, const std::string &req_path) {
    if (access(dir_name.c_str(), R_OK) == -1) {
        throw MustReturnHTTPStatus(403);
    }

    std::string html_string;
    html_string += gen_header(req_path);
    html_string += gen_body(dir_name, req_path);
    return html_string;
}
