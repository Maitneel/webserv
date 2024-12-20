#ifndef INCLUDE_CHUNKED_HPP_
#define INCLUDE_CHUNKED_HPP_

#include <string>
#include <map>
#include <exception>

// -------------------------- decode -------------------------- //

class ChunkDecoder {
 public:
    void AddBuffer(const std::string &buffer);
    const std::string &GetDecodedData();
    const bool &IsDecodeComplete();

    ChunkDecoder();
    ChunkDecoder(const std::string &buffer);

    class InvalidFormat : public std::exception {
     public:
        InvalidFormat() {};
        const char *what() const throw() {
            return "ChunkDecoder: Invalid Format";
        }
    };

    class ChunkSizeTooLarge : public std::exception {
     public:
        ChunkSizeTooLarge() {}
        const char *what() const throw() {
            return "ChunkDecoder: chunk-size too large";
        }
    };

 private: 
    bool find_last_chank_;
    bool find_last_crlf_;
    std::string decoded_data_;
    std::string chunked_data_;
    size_t chunked_length_;
    size_t remaining_length_;

    void DecodeChunk();
};

// -------------------------- encode -------------------------- //

std::string encode_chunk(const std::string &data);
#define LAST_CHANK "0\r\n"
// std::string encode_trailer_section(const std::multimap<std::string, std::string> &src);

#define LAST_CHUNKE_AND_EMPTY_TRAILER "0\r\n\r\n"
// 最後のchunkを作成して、その後の(RFC9112で定義されてる意味での)last-chunkとtrailer-sectionを同時に生成する関数 //
std::string encode_last_data_chunk_with_trailer(const std::string &data);


#endif // INCLUDE_CHUNKED_HPP_
