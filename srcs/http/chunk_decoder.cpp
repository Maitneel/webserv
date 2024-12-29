#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstring>

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

void ChunkDecoder::RemovePrevCRLF() {
    if (2 <= chunked_data_.length() && chunked_data_.at(0) == '\r' && chunked_data_.at(1) == '\n') {
        chunked_data_.erase(0, strlen("\r\n"));
    }
}

bool ChunkDecoder::IsIncludeSizeLine() {
    return (chunked_data_.find("\r\n") != std::string::npos);
}

void ChunkDecoder::ParseSizeLine() {
    try {
        chunked_length_ = safe_hex_to_sizet(chunked_data_);
        remaining_length_ = chunked_length_;
        if (chunked_length_ == 0) {
            find_last_chank_ = true;
            if (chunked_data_.find("\r\n\r\n") != std::string::npos) {
                find_last_crlf_ = true;
            }
            return;
        }
        chunked_data_.erase(0, chunked_data_.find("\r\n") + strlen("\r\n"));
    } catch (std::overflow_error &e) {
        throw ChunkDecoder::ChunkSizeTooLarge();
    } catch (std::runtime_error &e) {
        throw ChunkDecoder::InvalidFormat();
    }
}

void ChunkDecoder::DecodeChunk() {
    do {
        if (remaining_length_ == 0) {
            this->RemovePrevCRLF();
            if (!this->IsIncludeSizeLine()) {
                break;
            }
            this->ParseSizeLine();
            if (find_last_chank_) {
                break;
            }
        }
        decoded_data_ += chunked_data_.substr(0, remaining_length_);
        size_t decoded_length = std::min(chunked_data_.length(), remaining_length_);
        chunked_data_.erase(0, decoded_length);
        remaining_length_ -= decoded_length;
    } while (chunked_data_.length() && chunked_length_ != 0);
}

ChunkDecoder::ChunkDecoder() : find_last_chank_(false), find_last_crlf_(false), decoded_data_() , chunked_data_(), chunked_length_(0), remaining_length_(0) {
}

ChunkDecoder::ChunkDecoder(const std::string &buffer)  : find_last_chank_(false), find_last_crlf_(false), decoded_data_(), chunked_data_(buffer), chunked_length_(0), remaining_length_(0) {
    this->DecodeChunk();
}
