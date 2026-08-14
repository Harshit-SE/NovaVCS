#include "nova/core/search_engine.hpp"
#include <sstream>
#include <cctype>

namespace nova::search {

SearchEngine::SearchEngine() : root(std::make_unique<TrieNode>()) {}

void SearchEngine::insertTrie(const std::string& word, const std::string& reference) {
    TrieNode* current = root.get();
    for (char c : word) {
        if (!current->children.count(c)) {
            current->children[c] = std::make_unique<TrieNode>();
        }
        current = current->children[c].get();
    }
    current->isTerminal = true;
    current->references.push_back(reference);
}

void SearchEngine::indexFile(const std::string& filename, const std::string& content) {
    // 1. Add to Trie for filename prefix search
    insertTrie(filename, filename);

    // 2. Add to Inverted Index for content search
    std::istringstream stream(content);
    std::string word;
    while (stream >> word) {
        // Clean basic punctuation for better indexing
        word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        if (!word.empty()) {
            invertedIndex[word].push_back(filename);
        }
    }
}

void SearchEngine::indexCommit(const std::string& hash, const std::string& message) {
    exactStore[hash] = message;
    insertTrie(hash, hash); // Allow prefix search for short hashes
    
    std::istringstream stream(message);
    std::string word;
    while (stream >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        invertedIndex[word].push_back("commit:" + hash);
    }
}

void SearchEngine::indexBranch(const std::string& branchName) {
    insertTrie(branchName, "branch:" + branchName);
}

void SearchEngine::collectAllWords(TrieNode* node, const std::string& currentPrefix, std::vector<std::string>& results) {
    if (node->isTerminal) {
        results.push_back(currentPrefix);
    }
    for (const auto& [c, child] : node->children) {
        collectAllWords(child.get(), currentPrefix + c, results);
    }
}

std::vector<std::string> SearchEngine::prefixSearch(const std::string& prefix) {
    TrieNode* current = root.get();
    for (char c : prefix) {
        if (!current->children.count(c)) return {}; // Prefix not found
        current = current->children[c].get();
    }
    
    std::vector<std::string> results;
    collectAllWords(current, prefix, results);
    return results;
}

std::vector<std::string> SearchEngine::contentSearch(const std::string& word) {
    std::string lowerWord = word;
    std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);
    
    if (invertedIndex.count(lowerWord)) {
        return invertedIndex[lowerWord];
    }
    return {};
}

std::vector<std::string> SearchEngine::exactSearch(const std::string& key) {
    if (exactStore.count(key)) {
        return {exactStore[key]};
    }
    return {};
}

int SearchEngine::calculateLevenshteinDistance(const std::string& s1, const std::string& s2) {
    std::vector<std::vector<int>> dp(s1.size() + 1, std::vector<int>(s2.size() + 1));
    
    for (size_t i = 0; i <= s1.size(); i++) dp[i][0] = i;
    for (size_t j = 0; j <= s2.size(); j++) dp[0][j] = j;
    
    for (size_t i = 1; i <= s1.size(); i++) {
        for (size_t j = 1; j <= s2.size(); j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,       // Deletion
                dp[i][j - 1] + 1,       // Insertion
                dp[i - 1][j - 1] + cost // Substitution
            });
        }
    }
    return dp[s1.size()][s2.size()];
}

std::vector<std::string> SearchEngine::fuzzySearch(const std::string& query, int maxDistance) {
    std::vector<std::string> results;
    // For a real massive repo, this scans the Trie space or vocabulary list.
    // Scanning the inverted index keys here as a unified dictionary.
    for (const auto& [word, locations] : invertedIndex) {
        if (calculateLevenshteinDistance(query, word) <= maxDistance) {
            results.push_back(word);
        }
    }
    return results;
}

} // namespace nova::search
