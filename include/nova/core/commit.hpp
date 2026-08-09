#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <sstream>

namespace nova::core {

struct Commit {
    std::string oid;               // The SHA-256 hash of this commit object
    std::string tree_oid;          // The root directory state hash
    std::vector<std::string> parents; // DAG pointers to parent commits
    std::string author;
    std::string message;
    uint64_t timestamp;

    // Serialization for ObjectDB
    std::string serialize() const;
    
    // Deserialization from ObjectDB
    static Commit deserialize(const std::string& data);
    
    // Validates commit integrity
    bool isValid() const;
};

} // namespace nova::core
