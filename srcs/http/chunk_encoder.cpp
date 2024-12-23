#include <string>

#include "extend_stdlib.hpp"

std::string encode_chunk(const std::string &data) {
    std::string chunk = size_t_to_hex_string(data.length());
    chunk += "\r\n";
    chunk += data;
    chunk += "\r\n";
    return chunk;
}
// std::string encode_trailer_section(const std::multimap<std::string, std::string> &src);

// 最後のchunkを作成して、その後の(RFC9112で定義されてる意味での)last-chunkとtrailer-sectionを同時に生成する関数 //
std::string encode_last_data_chunk_with_trailer(const std::string &data) {
    std::string chunk = encode_chunk(data);
    chunk += "0\r\n";  // last-chunk;
    // notthing trailer section;
    chunk += "\r\n";   // CRLF;
    return chunk;
}
