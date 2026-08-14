#include <gtest/gtest.h>
#include "nova/core/search_engine.hpp"
#include <chrono>
#include <iostream>
#include <random>

using namespace nova::search;
using namespace std::chrono;

std::string generateRandomContent(int words) {
    std::string content;
    const std::vector<std::string> vocab = {"void", "int", "database", "postgres", "memory", "buffer", "commit", "engine", "pointer", "tree"};
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dist(0, vocab.size() - 1);
    
    for (int i = 0; i < words; ++i) {
        content += vocab[dist(gen)] + " ";
    }
    return content;
}

TEST(SearchEngineTest, PerformanceBenchmarks) {
    SearchEngine engine;
    int numFiles = 10000;
    
    std::cout << "\n--- Starting Search Engine Benchmarks ---\n";

    // 1. Benchmark Indexing
    auto startIdx = high_resolution_clock::now();
    for (int i = 0; i < numFiles; ++i) {
        engine.indexFile("src/module_" + std::to_string(i) + ".cpp", generateRandomContent(50));
    }
    engine.indexCommit("5fa5b8", "Implemented PERN stack infrastructure");
    engine.indexBranch("feature-postgres");
    
    auto endIdx = high_resolution_clock::now();
    std::cout << "[Index Build] 10,000 files indexed in: " 
              << duration_cast<milliseconds>(endIdx - startIdx).count() << " ms\n";

    // 2. Benchmark Prefix Search (Trie)
    auto startPref = high_resolution_clock::now();
    auto prefixResults = engine.prefixSearch("src/module_999");
    auto endPref = high_resolution_clock::now();
    
    std::cout << "[Prefix Search (Trie)] Query 'src/module_999' found " << prefixResults.size() 
              << " results in: " << duration_cast<microseconds>(endPref - startPref).count() << " µs\n";

    // 3. Benchmark Content Search (Inverted Index)
    auto startCont = high_resolution_clock::now();
    auto contentResults = engine.contentSearch("postgres");
    auto endCont = high_resolution_clock::now();
    
    std::cout << "[Content Search (InvIndex)] Query 'postgres' found " << contentResults.size() 
              << " references in: " << duration_cast<microseconds>(endCont - startCont).count() << " µs\n";

    // 4. Benchmark Fuzzy Search (Levenshtein)
    auto startFuzz = high_resolution_clock::now();
    auto fuzzyResults = engine.fuzzySearch("postgress", 2); // Typo test
    auto endFuzz = high_resolution_clock::now();
    
    std::cout << "[Fuzzy Search (Levenshtein)] Query 'postgress' found " << fuzzyResults.size() 
              << " suggestions in: " << duration_cast<microseconds>(endFuzz - startFuzz).count() << " µs\n";
              
    std::cout << "----------------------------------------\n";
}
