#include "nova/storage/object_db.hpp"
#include "nova/storage/compressor.hpp"
#include "nova/crypto/sha256.hpp"
#include "nova/utils/fs_utils.hpp"
#include "nova/core/error.hpp"
#include <sstream>

namespace nova::storage {

ObjectDB::ObjectDB(std::filesystem::path repoRoot) 
    : objectsDir_(std::move(repoRoot) / ".nova" / "objects") {}

std::string ObjectDB::writeObject(const std::string& type, const std::string& content) {
    // 1. Construct standard object format: "<type> <size>\0<content>"
    std::string header = type + " " + std::to_string(content.size());
    std::string serializedObj = header + '\0' + content;

    // 2. Hash the serialized object to get the OID
    std::string oid = crypto::SHA256::hash(serializedObj);

    // 3. Determine the storage path (.nova/objects/ab/cdef...)
    std::string dirStr = oid.substr(0, 2);
    std::string fileStr = oid.substr(2);
    auto targetDir = objectsDir_ / dirStr;
    auto targetFile = targetDir / fileStr;

    // 4. If object already exists, do not overwrite (CAS property)
    if (utils::FSUtils::exists(targetFile)) {
        return oid;
    }

    // 5. Compress and write
    std::string compressedObj = Compressor::compress(serializedObj);
    utils::FSUtils::createDirectory(targetDir);
    utils::FSUtils::createFile(targetFile, compressedObj);

    return oid;
}

std::pair<std::string, std::string> ObjectDB::readObject(const std::string& oid) {
    // 1. Locate the file
    std::string dirStr = oid.substr(0, 2);
    std::string fileStr = oid.substr(2);
    auto targetFile = objectsDir_ / dirStr / fileStr;

    if (!utils::FSUtils::exists(targetFile)) {
        throw core::NovaException("Object not found: " + oid);
    }

    // 2. Read and Decompress
    std::string compressedData = utils::FSUtils::readFile(targetFile);
    std::string decompressedData = Compressor::decompress(compressedData);

    // 3. Parse header "<type> <size>\0<content>"
    size_t spacePos = decompressedData.find(' ');
    size_t nullPos = decompressedData.find('\0', spacePos);

    if (spacePos == std::string::npos || nullPos == std::string::npos) {
        throw core::NovaException("Corrupt object header for OID: " + oid);
    }

    std::string type = decompressedData.substr(0, spacePos);
    std::string content = decompressedData.substr(nullPos + 1);

    // 4. Verify Integrity (re-hash and check against requested OID)
    std::string verifyOid = crypto::SHA256::hash(decompressedData);
    if (verifyOid != oid) {
        throw core::NovaException("Object integrity verification failed for OID: " + oid);
    }

    return {type, content};
}

} // namespace nova::storage
