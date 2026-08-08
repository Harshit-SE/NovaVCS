#include "IndexManager.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

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

void IndexManager::addFile(const std::string& path, const std::string& hash) {
    if (ignoreEngine.isIgnored(path)) return;

    fs::path fullPath = fs::path(repoRoot) / path;
    if (!fs::exists(fullPath)) return;

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
        // Update mtime and size in case it was modified
        fs::path fullPath = fs::path(repoRoot) / path;
        if (fs::exists(fullPath)) {
            it->second->entry->size = fs::file_size(fullPath);
            it->second->entry->mtime = fs::last_write_time(fullPath);
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

std::string IndexManager::computeHashDummy(const std::string& path) const {
    // In production, this calls your Phase 2 SHA-256 CAS engine.
    // Returning a dummy hash based on file size for rename detection logic testing.
    fs::path fullPath = fs::path(repoRoot) / path;
    return "hash_" + std::to_string(fs::file_size(fullPath));
}

RepoStatus IndexManager::generateStatus() {
    RepoStatus status;
    std::unordered_map<std::string, bool> diskFiles;

    // 1. Scan Working Directory
    for (const auto& entry : fs::recursive_directory_iterator(repoRoot)) {
        if (entry.is_directory()) continue;
        
        std::string relPath = fs::relative(entry.path(), repoRoot).string();
        // Standardize separators for Windows/Linux consistency
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

    // 2. Scan Index for Deleted Files
    for (const auto& [path, node] : fastLookup) {
        if (node->entry.has_value() && diskFiles.find(path) == diskFiles.end()) {
            status.deleted.push_back(path);
        }
    }

    // 3. Rename Detection Heuristic (O(N) using size/hash matching)
    std::unordered_map<std::string, std::string> untrackedHashes;
    for (auto it = status.untracked.begin(); it != status.untracked.end();) {
        std::string hash = computeHashDummy(*it);
        untrackedHashes[hash] = *it;
        ++it;
    }

    for (auto it = status.deleted.begin(); it != status.deleted.end();) {
        std::string deletedHash = fastLookup[*it]->entry->hash;
        if (untrackedHashes.find(deletedHash) != untrackedHashes.end()) {
            std::string newPath = untrackedHashes[deletedHash];
            status.renamed.push_back({*it, newPath});
            
            // Remove from untracked and deleted since it's a rename
            status.untracked.erase(std::remove(status.untracked.begin(), status.untracked.end(), newPath), status.untracked.end());
            it = status.deleted.erase(it);
        } else {
            ++it;
        }
    }

    return status;
}

// Binary serialization methods omitted for brevity, but would use std::fstream 
// to write the fastLookup map iteratively for O(1) loading.
void IndexManager::saveIndex(const std::string& indexPath) const {}
void IndexManager::loadIndex(const std::string& indexPath) {}
