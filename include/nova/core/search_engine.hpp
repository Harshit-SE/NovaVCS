#ifndef NOVA_SEARCH_ENGINE_HPP
#define NOVA_SEARCH_ENGINE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

namespace nova::search {

struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    bool isTerminal = false;
    std::vector<std::string> references; // What this prefix maps to (e.g., file paths)
};

class SearchEngine {
private:
    std::unique_ptr<TrieNode> root;
    
    // Inverted Index: Word -> List of FilePaths/CommitHashes
    std::unordered_map<std::string, std::vector<std::string>> invertedIndex;
    
    // Hash Table: Exact key -> Metadata
    std::unordered_map<std::string, std::string> exactStore;

    // Helper for Trie
    void insertTrie(const std::string& word, const std::string& reference);
    void collectAllWords(TrieNode* node, const std::string& currentPrefix, std::vector<std::string>& results);
    
    // Helper for Fuzzy Search
    int calculateLevenshteinDistance(const std::string& s1, const std::string& s2);

public:
    SearchEngine();

    // Indexing
    void indexFile(const std::string& filename, const std::string& content);
    void indexCommit(const std::string& hash, const std::string& message);
    void indexBranch(const std::string& branchName);

    // Searching
    std::vector<std::string> prefixSearch(const std::string& prefix); // Auto-complete
    std::vector<std::string> contentSearch(const std::string& word);  // Inverted Index
    std::vector<std::string> exactSearch(const std::string& key);     // Hash Table
    std::vector<std::string> fuzzySearch(const std::string& query, int maxDistance = 2);
};

} // namespace nova::search

#endif // NOVA_SEARCH_ENGINE_HPP
