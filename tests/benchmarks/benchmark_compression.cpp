#include <gtest/gtest.h>
#include "nova/compression/compression_engine.hpp"
#include <chrono>
#include <iostream>

using namespace nova::compression;
using namespace std::chrono;

TEST(CompressionEngineTest, Benchmarks) {
    std::string basePayload;
    for (int i = 0; i < 50000; ++i) basePayload += "int main() { return 0; } ";
    
    std::string modifiedPayload = basePayload + " void newFeature() {}";

    std::cout << "\n--- Compression Engine Benchmarks ---\n";

    // Benchmark Huffman Coding
    HuffmanCoder huffman;
    auto startHuff = high_resolution_clock::now();
    std::string encoded = huffman.encode(basePayload);
    auto endHuff = high_resolution_clock::now();
    
    size_t originalBytes = basePayload.size();
    size_t huffmanBytes = encoded.size() / 8; // Bit string to bytes

    std::cout << "[Huffman Coder]\n";
    std::cout << "  Time  : " << duration_cast<milliseconds>(endHuff - startHuff).count() << " ms\n";
    std::cout << "  Size  : " << originalBytes << " B -> " << huffmanBytes << " B\n";

    // Benchmark Delta Compression
    auto startDelta = high_resolution_clock::now();
    std::string delta = DeltaCompressor::computeDelta(basePayload, modifiedPayload);
    auto endDelta = high_resolution_clock::now();

    std::cout << "[Delta Compressor]\n";
    std::cout << "  Time  : " << duration_cast<microseconds>(endDelta - startDelta).count() << " µs\n";
    std::cout << "  Size  : " << modifiedPayload.size() << " B -> " << delta.size() << " B (Instruction format)\n";
    std::cout << "----------------------------------------\n";
}
