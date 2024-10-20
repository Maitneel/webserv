#ifndef INCLUDE_SIMPLE_DB_HPP_
#define INCLUDE_SIMPLE_DB_HPP_

#include <string>
#include <map>
#include <vector>
#include <utility>

class SimpleDB {
 private:
    std::map<std::string, std::string> data_;
    const std::string filename_;

    std::pair<std::string, std::string> parse_data_line(const std::string &s);
    std::string make_data_line(const std::pair<std::string, std::string> &src);

 public:
    SimpleDB(const std::string &file_path);
    ~SimpleDB();

    void add(const std::string &key, const std::string &value);
    void remove(const std::string &key);
    void update(const std::string &key, const std::string &value);
    const std::string &get(const std::string &key);

    std::vector<std::string> get_include_key();
};

#endif // INCLUDE_SIMPLE_DB_HPP_
