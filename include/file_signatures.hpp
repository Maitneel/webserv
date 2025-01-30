#ifndef INCLUDE_FILE_SIGNATURES_HPP_
#define INCLUDE_FILE_SIGNATURES_HPP_

#include <string>
#include <vector>

#define WILD_CARD_NUMBER -1

class FileSignatures {
 public:
    FileSignatures();
    ~FileSignatures();

    const std::string &GetMIMEType(const std::string &file_name) const;

 private:
    static const std::string octed_;
    int max_length_;
    std::vector<std::pair<std::vector<int>, std::string> > magic_numbers;

    bool IsIncludeInFront(const std::vector<int> &dest, const std::vector<int> &src) const;
};

#endif  // INCLUDE_FILE_SIGNATURES_HPP_
