#include <unistd.h>
#include <sys/fcntl.h>
#include <cstring>

#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include "file_signatures.hpp"


// image //
const int png_magic[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
const int jpeg0[] = {0xFF, 0xD8, 0xFF, 0xDB};
const int jpeg1[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01};
const int jpee2[] = {0xFF, 0xD8, 0xFF, 0xEE};
const int jpeg3[] = {0xFF, 0xD8, 0xFF, 0xE1, WILD_CARD_NUMBER, WILD_CARD_NUMBER, 0x45, 0x78, 0x69, 0x66, 0x00, 0x0};
const int jpeg4[] = {0xFF, 0xD8, 0xFF, 0xE0};

// pdf //
const int pdf[] = {0x25, 0x50, 0x44, 0x46, 0x2D};

// audio //
const int wav[] = {0x52, 0x49, 0x46, 0x46, WILD_CARD_NUMBER, WILD_CARD_NUMBER, WILD_CARD_NUMBER, WILD_CARD_NUMBER, 0x57, 0x41, 0x56, 0x45};
const int mp3_0[] = {0xFF, 0xFB};
const int mp3_1[] = {0xFF, 0xF3};
const int mp3_2[] = {0xFF, 0xF2};
const int mp3_3[] = {0x49, 0x44, 0x33};

// video //
const int mp4_0[] = {0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D};
const int mp4_1[] = {0x66, 0x74, 0x79, 0x70, 0x4D, 0x53, 0x4E, 0x56};

// submachine-gun //
const int mp5[] = {0x48, 0x4B, 0x20, 0x4D, 0x50, 0x35};

#define make_magic_type_pair(magic, type) (std::make_pair(std::vector<int>(magic, magic + sizeof(magic) / sizeof(int)), type))

const std::string FileSignatures::octed_ = "application/octet-stream";

FileSignatures::FileSignatures() : max_length_(0) {
    magic_numbers.push_back(make_magic_type_pair(png_magic, "image/png"));
    magic_numbers.push_back(make_magic_type_pair(jpeg0, "image/jpeg"));
    magic_numbers.push_back(make_magic_type_pair(jpeg1, "image/jpeg"));
    magic_numbers.push_back(make_magic_type_pair(jpee2, "image/jpeg"));
    magic_numbers.push_back(make_magic_type_pair(jpeg3, "image/jpeg"));
    magic_numbers.push_back(make_magic_type_pair(jpeg4, "image/jpeg"));
    magic_numbers.push_back(make_magic_type_pair(pdf, "application/pdf"));
    magic_numbers.push_back(make_magic_type_pair(wav, "audio/wav"));
    magic_numbers.push_back(make_magic_type_pair(mp3_0, "audio/mpeg"));
    magic_numbers.push_back(make_magic_type_pair(mp3_1, "audio/mpeg"));
    magic_numbers.push_back(make_magic_type_pair(mp3_2, "audio/mpeg"));
    magic_numbers.push_back(make_magic_type_pair(mp3_3, "audio/mpeg"));
    magic_numbers.push_back(make_magic_type_pair(mp4_0, "video/mp4"));
    magic_numbers.push_back(make_magic_type_pair(mp4_1, "video/mp4"));
    magic_numbers.push_back(make_magic_type_pair(mp5, "x-submachine-gun/x-mp5"));

    for (size_t i = 0; i < magic_numbers.size(); i++) {
        max_length_ = std::max(max_length_, static_cast<int>(magic_numbers[i].first.size()));
    }
}

FileSignatures::~FileSignatures() {
}

const std::string &FileSignatures::GetMIMEType(const std::string &file_name) const {
    std::ifstream ifs(file_name.c_str());
    if (!ifs) {
        return this->octed_;
    }

    std::vector<int> head;
    unsigned char c;
    for (int i = 0; i < max_length_ && !ifs.eof(); i++) {
        c = ifs.get();
        head.push_back(static_cast<int>(c));
    }
    ifs.close();


    for (size_t i = 0; i < magic_numbers.size(); i++) {
        if (this->IsIncludeInFront(head, magic_numbers[i].first)) {
            return magic_numbers[i].second;
        }
    }
    return this->octed_;
}

bool FileSignatures::IsIncludeInFront(const std::vector<int> &dest, const std::vector<int> &src) const {
    if (dest.size() < src.size()) {
        return false;
    }

    for (size_t i = 0; i < src.size(); i++) {
        if (dest[i] != src[i] && src[i] != WILD_CARD_NUMBER) {
            return false;
        }
    }
    return true;
}


/*
#include <iostream>
using namespace std;
int main(int argc, char **argv) {
    FileSignatures fs;
    cerr << argv[1] << ": " << fs.GetMIMEType(argv[1]) << endl;
}
// */
