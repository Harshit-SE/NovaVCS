/**
 * @file sha256.hpp
 * @brief SHA-256 Cryptographic Hashing using modern OpenSSL EVP API.
 */
#pragma once
#include <string>
#include <string_view>

namespace nova::crypto {

class SHA256 {
public:
    /**
     * @brief Computes the SHA-256 hash of a given string payload.
     * @param data The input data to hash.
     * @return 40-character hexadecimal string representing the hash.
     */
    static std::string hash(std::string_view data);
};

} // namespace nova::crypto
