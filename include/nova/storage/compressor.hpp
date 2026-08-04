/**
 * @file compressor.hpp
 * @brief Zlib deflate/inflate wrappers for object compression.
 */
#pragma once
#include <string>

namespace nova::storage {

class Compressor {
public:
    static std::string compress(const std::string& data);
    static std::string decompress(const std::string& compressedData);
};

} // namespace nova::storage
