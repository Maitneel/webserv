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

#include <iostream>
using std::cerr;
using std::endl;

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

static std::string gen_index(const std::string &dir_name, const std::string &req_path) {
    DIR *dir = opendir(dir_name.c_str());
    if (dir == NULL) {
        throw MustReturnHTTPStatus(500);
    }

    std::set<FileStat> dir_info;
    struct dirent *file_element;
    while ((file_element = readdir(dir)) != NULL) {
        std::string file_name = file_element->d_name;
        // TODO(maitneel): dir_nameは常に'/'で終わるべきなので本来"/"を追加する必要はない //
        std::string file_path = dir_name + "/" + file_name;
        struct stat file_info;
        cerr << file_path << endl;
        if (file_name.at(0) == '.') {
            continue;
        }
        if (stat(file_path.c_str(), &file_info) != 0) {
            cerr << "throw: " << file_path << endl;
            throw MustReturnHTTPStatus(500);
        }
        if ((file_info.st_mode & S_IFDIR)) {
            file_name += '/';
            dir_info.insert(FileStat(file_name, req_path + file_name, file_info.st_size, true));
        } else {
            dir_info.insert(FileStat(file_name, req_path + file_name, file_info.st_size, false));
        }
    }
    std::stringstream data;
    data << "<h1>Index of " << req_path << "</h1>\n<hr>\n<pre>\n";
    data << "<a href=\"" << req_path << "\">" << "../" << "</a>\n";
    for (std::set<FileStat>::iterator it = dir_info.begin(); it != dir_info.end(); it++) {
        std::string file_name = it->name;
        if (AUTOINDEX_WIDTH < file_name.length()) {
            file_name = file_name.substr(0, AUTOINDEX_WIDTH - 3);
            file_name += "..>";
        }
        data << "<a href=\"" << it->path << "\">" << file_name << "</a>" << std::setw(AUTOINDEX_WIDTH - file_name.length()) << "" << ' ';
        if (it->is_dir) {
            data << "-" << "\n";
        } else {
            data << it->size << "\n";
        }
    }
    data << "</pre><hr>";

    return data.str();
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

    std::string data;
    data += gen_header(req_path);
    data += gen_body(dir_name, req_path);
    return data;
}
