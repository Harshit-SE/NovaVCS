#include "nova/crypto/sha256.hpp"
#include "nova/core/error.hpp"
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

namespace nova::crypto {

std::string SHA256::hash(std::string_view data) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr) {
        throw core::NovaException("Failed to create OpenSSL EVP Context");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw core::NovaException("Failed to initialize SHA-256 Digest");
    }

    if (EVP_DigestUpdate(context, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(context);
        throw core::NovaException("Failed to update SHA-256 Digest");
    }

    unsigned char hashResult[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    if (EVP_DigestFinal_ex(context, hashResult, &lengthOfHash) != 1) {
        EVP_MD_CTX_free(context);
        throw core::NovaException("Failed to finalize SHA-256 Digest");
    }

    EVP_MD_CTX_free(context);

    std::stringstream hexStream;
    for (unsigned int i = 0; i < lengthOfHash; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)hashResult[i];
    }

    return hexStream.str();
}

} // namespace nova::crypto
