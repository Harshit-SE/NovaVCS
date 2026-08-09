#ifndef INDEX_MANAGER_HPP
#define INDEX_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include "IgnoreEngine.hpp"

struct IndexEntry {
    std::string path;
    std::string hash;
    uint64_t size;
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
    std::vector<std::pair<std::string, std::string>> renamed;
};

class IndexManager {
public:
    explicit IndexManager(const std::string& rootDir);

    // FIX: Single argument signature
    void addFile(const std::string& path);
    void removeFile(const std::string& path);
    void stageFile(const std::string& path);
    void unstageFile(const std::string& path);
    
    std::optional<IndexEntry> getMetadata(const std::string& path) const;
    RepoStatus generateStatus();

    void saveIndex(const std::string& indexPath) const;
    void loadIndex(const std::string& indexPath);

private:
    std::string repoRoot;
    std::unique_ptr<TrieNode> trieRoot;
    std::unordered_map<std::string, TrieNode*> fastLookup;
    IgnoreEngine ignoreEngine;

    std::vector<std::string> splitPath(const std::string& path) const;
    TrieNode* insertPath(const std::string& path);
    std::string computeHashReal(const std::string& path) const;
};

#endif // INDEX_MANAGER_HPP
