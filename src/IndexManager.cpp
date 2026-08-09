#include "IndexManager.hpp"
#include "nova/crypto/sha256.hpp" 
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>

namespace fs = std::filesystem;

IndexManager::IndexManager(const std::string& rootDir) : repoRoot(rootDir) {
    trieRoot = std::make_unique<TrieNode>();
    fs::path ignorePath = fs::path(repoRoot) / ".novaignore";
    if (fs::exists(ignorePath)) {
        ignoreEngine.loadRules(ignorePath.string());
    }
}

std::vector<std::string> IndexManager::splitPath(const std::string& path) const {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string item;
    while (std::getline(ss, item, '/')) {
        if (!item.empty()) parts.push_back(item);
    }
    return parts;
}

TrieNode* IndexManager::insertPath(const std::string& path) {
    auto parts = splitPath(path);
    TrieNode* current = trieRoot.get();
    for (const auto& part : parts) {
        if (current->children.find(part) == current->children.end()) {
            current->children[part] = std::make_unique<TrieNode>();
        }
        current = current->children[part].get();
    }
    fastLookup[path] = current;
    return current;
}

std::string IndexManager::computeHashReal(const std::string& path) const {
    fs::path fullPath = fs::path(repoRoot) / path;
    
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) return "";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    std::string payload = "blob " + std::to_string(content.size()) + '\0' + content;
    return nova::crypto::SHA256::hash(payload);
}

void IndexManager::addFile(const std::string& path) {
    if (ignoreEngine.isIgnored(path)) return;

    fs::path fullPath = fs::path(repoRoot) / path;
    if (!fs::exists(fullPath)) return;

    std::string hash = computeHashReal(path);
    if (hash.empty()) return;

    TrieNode* node = insertPath(path);
    IndexEntry entry;
    entry.path = path;
    entry.hash = hash;
    entry.size = fs::file_size(fullPath);
    entry.mtime = fs::last_write_time(fullPath);
    entry.is_staged = true;

    node->entry = entry;
}

void IndexManager::removeFile(const std::string& path) {
    auto it = fastLookup.find(path);
    if (it != fastLookup.end()) {
        it->second->entry = std::nullopt;
        fastLookup.erase(it);
    }
}

void IndexManager::stageFile(const std::string& path) {
    auto it = fastLookup.find(path);
    if (it != fastLookup.end() && it->second->entry) {
        it->second->entry->is_staged = true;
        fs::path fullPath = fs::path(repoRoot) / path;
        if (fs::exists(fullPath)) {
            it->second->entry->size = fs::file_size(fullPath);
            it->second->entry->mtime = fs::last_write_time(fullPath);
            it->second->entry->hash = computeHashReal(path);
        }
    }
}

void IndexManager::unstageFile(const std::string& path) {
    auto it = fastLookup.find(path);
    if (it != fastLookup.end() && it->second->entry) {
        it->second->entry->is_staged = false;
    }
}

std::optional<IndexEntry> IndexManager::getMetadata(const std::string& path) const {
    auto it = fastLookup.find(path);
    if (it != fastLookup.end()) {
        return it->second->entry;
    }
    return std::nullopt;
}

RepoStatus IndexManager::generateStatus() {
    RepoStatus status;
    std::unordered_map<std::string, bool> diskFiles;

    for (const auto& entry : fs::recursive_directory_iterator(repoRoot)) {
        if (entry.is_directory()) continue;
        
        std::string relPath = fs::relative(entry.path(), repoRoot).string();
        std::replace(relPath.begin(), relPath.end(), '\\', '/');

        if (ignoreEngine.isIgnored(relPath)) continue;
        diskFiles[relPath] = true;

        auto it = fastLookup.find(relPath);
        if (it == fastLookup.end() || !it->second->entry.has_value()) {
            status.untracked.push_back(relPath);
        } else {
            auto& idxEntry = it->second->entry.value();
            auto currentMtime = fs::last_write_time(entry.path());
            auto currentSize = fs::file_size(entry.path());

            if (idxEntry.size != currentSize || idxEntry.mtime != currentMtime) {
                status.modified.push_back(relPath);
            } else if (idxEntry.is_staged) {
                status.staged.push_back(relPath);
            }
        }
    }

    for (const auto& [path, node] : fastLookup) {
        if (node->entry.has_value() && diskFiles.find(path) == diskFiles.end()) {
            status.deleted.push_back(path);
        }
    }

    std::unordered_map<std::string, std::string> untrackedHashes;
    for (const auto& file : status.untracked) {
        std::string hash = computeHashReal(file);
        if (!hash.empty()) {
            untrackedHashes[hash] = file;
        }
    }

    std::vector<std::string> newDeleted;
    std::unordered_set<std::string> renamedUntrackedFiles;

    for (const auto& deletedPath : status.deleted) {
        auto lookupIt = fastLookup.find(deletedPath);
        
        if (lookupIt == fastLookup.end() || lookupIt->second == nullptr || !lookupIt->second->entry.has_value()) {
            newDeleted.push_back(deletedPath);
            continue;
        }
        
        std::string deletedHash = lookupIt->second->entry->hash;
        
        if (!deletedHash.empty() && untrackedHashes.find(deletedHash) != untrackedHashes.end()) {
            std::string newPath = untrackedHashes[deletedHash];
            status.renamed.push_back({deletedPath, newPath});
            
            renamedUntrackedFiles.insert(newPath);
            untrackedHashes.erase(deletedHash); 
        } else {
            newDeleted.push_back(deletedPath);
        }
    }

    std::vector<std::string> newUntracked;
    for (const auto& file : status.untracked) {
        if (renamedUntrackedFiles.find(file) == renamedUntrackedFiles.end()) {
            newUntracked.push_back(file);
        }
    }

    status.deleted = newDeleted;
    status.untracked = newUntracked;

    return status;
}

void IndexManager::saveIndex(const std::string& indexPath) const {
    std::ofstream out(indexPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;

    size_t count = 0;
    for (const auto& [path, node] : fastLookup) {
        if (node->entry.has_value()) count++;
    }

    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& [path, node] : fastLookup) {
        if (!node->entry.has_value()) continue;
        const auto& entry = node->entry.value();

        size_t pathLen = entry.path.size();
        out.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
        out.write(entry.path.data(), pathLen);

        size_t hashLen = entry.hash.size();
        out.write(reinterpret_cast<const char*>(&hashLen), sizeof(hashLen));
        out.write(entry.hash.data(), hashLen);

        out.write(reinterpret_cast<const char*>(&entry.size), sizeof(entry.size));

        uint64_t mtime_val = entry.mtime.time_since_epoch().count();
        out.write(reinterpret_cast<const char*>(&mtime_val), sizeof(mtime_val));

        out.write(reinterpret_cast<const char*>(&entry.is_staged), sizeof(entry.is_staged));
    }
}

void IndexManager::loadIndex(const std::string& indexPath) {
    std::ifstream in(indexPath, std::ios::binary);
    if (!in.is_open()) return;

    size_t count = 0;
    if (!in.read(reinterpret_cast<char*>(&count), sizeof(count))) return;

    for (size_t i = 0; i < count; ++i) {
        IndexEntry entry;
        
        size_t pathLen = 0;
        in.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));
        entry.path.resize(pathLen);
        in.read(&entry.path[0], pathLen);

        size_t hashLen = 0;
        in.read(reinterpret_cast<char*>(&hashLen), sizeof(hashLen));
        entry.hash.resize(hashLen);
        in.read(&entry.hash[0], hashLen);

        in.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));

        uint64_t mtime_val = 0;
        in.read(reinterpret_cast<char*>(&mtime_val), sizeof(mtime_val));
        entry.mtime = std::filesystem::file_time_type(std::filesystem::file_time_type::duration(mtime_val));

        in.read(reinterpret_cast<char*>(&entry.is_staged), sizeof(entry.is_staged));

        TrieNode* node = insertPath(entry.path);
        node->entry = entry;
    }
}
