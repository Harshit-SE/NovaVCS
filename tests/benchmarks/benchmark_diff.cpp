#include <gtest/gtest.h>
#include "nova/core/diff_engine.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <random>

using namespace nova::diff;
using namespace std::chrono;

std::string generateRandomLines(int numLines, int mutationRate) {
    std::string result;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> charDist('a', 'z');
    std::uniform_int_distribution<> mutDist(1, 100);
    
    std::string baseLine = "const std::string base_pattern_for_testing = result;";
    
    for (int i = 0; i < numLines; i++) {
        if (mutDist(gen) <= mutationRate) {
            result += "mutated_line_var_" + std::to_string(gen()) + ";\n";
        } else {
            result += baseLine + "\n";
        }
    }
    return result;
}

void runBenchmark(const std::string& name, int lines, int mutationRate) {
    std::string oldText = generateRandomLines(lines, 0); 
    std::string newText = generateRandomLines(lines, mutationRate);

    auto start = high_resolution_clock::now();
    
    DiffResult result = DiffEngine::computeMyersDiff(oldText, newText);
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    std::cout << "[Benchmark] " << name << "\n"
              << "  Lines compared: " << lines << " vs " << lines << "\n"
              << "  Mutation Rate:  " << mutationRate << "%\n"
              << "  Edits found:    +" << result.additions << " -" << result.deletions << "\n"
              << "  Time taken:     " << duration << " ms\n"
              << "----------------------------------------\n";
}

TEST(DiffEngineTest, PerformanceBenchmarks) {
    std::cout << "\nStarting Diff Engine Benchmarks...\n\n";
    runBenchmark("Small File (Identical)", 1000, 0);
    runBenchmark("Small File (10% Diverged)", 1000, 10);
    runBenchmark("Medium File (5% Diverged)", 5000, 5);
    runBenchmark("Large File Stress Test (2% Diverged)", 10000, 2);
}
