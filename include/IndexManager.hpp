#ifndef NOVA_INDEX_MANAGER_HPP
#define NOVA_INDEX_MANAGER_HPP

#include "IgnoreEngine.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>
#include <filesystem>
#include <chrono>

struct IndexEntry {
    std::string path;
    std::string hash;      // SHA-256 hash of the file content
    uintmax_t size;
    std::filesystem::file_time_type mtime;
    bool is_staged;
};

struct TrieNode {
    std::unordered_map<std::string, std::unique_ptr<TrieNode>> children;
    std::optional<IndexEntry> entry;
};

struct RepoStatus {
    std::vector<std::string> staged;
    std::vector<std::string> modified;
    std::vector<std::string> untracked;
    std::vector<std::string> deleted;
    std::vector<std::pair<std::string, std::string>> renamed; // <old_path, new_path>
};

class IndexManager {
public:
    IndexManager(const std::string& rootDir);

    // Core File Operations
    void addFile(const std::string& path, const std::string& hash);
    void removeFile(const std::string& path);
    void stageFile(const std::string& path);
    void unstageFile(const std::string& path);
    
    // Metadata & Lookups
    std::optional<IndexEntry> getMetadata(const std::string& path) const;
    RepoStatus generateStatus();

    // I/O Operations for .nova/index
    void saveIndex(const std::string& indexPath) const;
    void loadIndex(const std::string& indexPath);

private:
    std::string repoRoot;
    std::unique_ptr<TrieNode> trieRoot;
    std::unordered_map<std::string, TrieNode*> fastLookup;
    IgnoreEngine ignoreEngine;

    std::vector<std::string> splitPath(const std::string& path) const;
    TrieNode* insertPath(const std::string& path);
    std::string computeHashDummy(const std::string& path) const; // Placeholder for Phase 2 CAS hook
};

#endif // NOVA_INDEX_MANAGER_HPP
