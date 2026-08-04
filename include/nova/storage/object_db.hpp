/**
 * @file object_db.hpp
 * @brief Manages reading and writing to the Content-Addressable Storage.
 */
#pragma once
#include <string>
#include <filesystem>
#include <utility>

namespace nova::storage {

/**
 * @class ObjectDB
 * @brief Interfaces with the `.nova/objects` directory.
 */
class ObjectDB {
public:
    explicit ObjectDB(std::filesystem::path repoRoot);

    /**
     * @brief Hashes, compresses, and stores an object to disk.
     * @param type "blob", "tree", or "commit"
     * @param content The raw data
     * @return The 40-character SHA-256 hash (OID)
     */
    std::string writeObject(const std::string& type, const std::string& content);

    /**
     * @brief Retrieves and decompresses an object from disk.
     * @param oid The 40-character SHA-256 hash.
     * @return A pair containing the object type and the raw content.
     */
    std::pair<std::string, std::string> readObject(const std::string& oid);

private:
    std::filesystem::path objectsDir_;
};

} // namespace nova::storage
