#include <string>
#include <algorithm>
#include <stdexcept>

#include "chunked.hpp"
#include "extend_stdlib.hpp"

void ChunkDecoder::AddBuffer(const std::string &buffer) {
    chunked_data_ += buffer;
    this->DecodeChunk();
}

const std::string &ChunkDecoder::GetDecodedData() {
    return decoded_data_;
}

const bool &ChunkDecoder::IsDecodeComplete() {
    return find_last_crlf_;
}

void ChunkDecoder::DecodeChunk() {
    do {
        if (remaining_length_ == 0) {
            try {
                chunked_length_ = safe_hex_to_sizet(chunked_data_);
                remaining_length_ = chunked_length_;
            } catch (std::overflow_error &e) {
                throw ChunkDecoder::InvalidFormat();
            } catch (std::runtime_error &e) {
                throw ChunkDecoder::ChunkSizeTooLarge();
            }
        }
        decoded_data_ += chunked_data_.substr(0, remaining_length_);
        size_t decoded_length = std::min(chunked_data_.length(), remaining_length_);
        chunked_data_.erase(0, remaining_length_);
        remaining_length_ -= decoded_length;
    } while (chunked_data_.length());
}

ChunkDecoder::ChunkDecoder() : find_last_chank_(false), find_last_crlf_(false), decoded_data_() , chunked_data_(), chunked_length_(0), remaining_length_(0) {
}

ChunkDecoder::ChunkDecoder(const std::string &buffer)  : find_last_chank_(false), find_last_crlf_(false), decoded_data_(), chunked_data_(buffer), chunked_length_(0), remaining_length_(0) {
    this->DecodeChunk();
}
