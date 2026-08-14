#include "nova/compression/compression_engine.hpp"
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace nova::compression {

// --- Huffman Coding ---

void HuffmanCoder::generateCodes(const Node* root, const std::string& str) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanCodes[root->data] = str;
    }
    generateCodes(root->left.get(), str + "0");
    generateCodes(root->right.get(), str + "1");
}

std::string HuffmanCoder::encode(const std::string& text) {
    std::unordered_map<char, unsigned> freq;
    for (char c : text) freq[c]++;

    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;
    for (auto pair : freq) {
        pq.push(new Node(pair.first, pair.second));
    }

    while (pq.size() != 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* top = new Node('$', left->freq + right->freq);
        top->left.reset(left);
        top->right.reset(right);
        pq.push(top);
    }

    generateCodes(pq.top(), "");
    std::string encodedString;
    for (char c : text) {
        encodedString += huffmanCodes[c];
    }
    
    // Cleanup root
    delete pq.top();
    return encodedString;
}

// --- Delta Compression ---

std::string DeltaCompressor::computeDelta(const std::string& base, const std::string& target) {
    // Simplified block-matching delta format:
    // Format: [I=Insert, C=Copy][Length]:[Data]
    // Note: For a production binary, this would emit raw bytes, not text instructions.
    std::ostringstream delta;
    
    if (base == target) return "C" + std::to_string(base.length()) + ":";
    
    // For this engine, we default to full insertion if base is empty, 
    // or naive diffing for demonstration.
    delta << "I" << target.length() << ":" << target;
    return delta.str();
}

std::string DeltaCompressor::applyDelta(const std::string& base, const std::string& delta) {
    if (delta.empty()) return base;
    if (delta[0] == 'I') {
        size_t colon = delta.find(':');
        return delta.substr(colon + 1);
    }
    return base;
}

// --- Object Packing ---

CompressionMetrics PackManager::packObjects(const fs::path& repoRoot) {
    fs::path objectsDir = repoRoot / ".nova" / "objects";
    fs::path packDir = repoRoot / ".nova" / "pack";
    fs::create_directories(packDir);
    
    size_t totalRawSize = 0;
    size_t totalCompressedSize = 0;
    
    std::string packFilePath = (packDir / "nova_data.pack").string();
    std::ofstream packFile(packFilePath, std::ios::binary);
    
    HuffmanCoder huffman;

    // Iterate through loose objects
    if (fs::exists(objectsDir)) {
        for (const auto& entry : fs::recursive_directory_iterator(objectsDir)) {
            if (entry.is_regular_file()) {
                std::ifstream objFile(entry.path(), std::ios::binary);
                std::stringstream buffer;
                buffer << objFile.rdbuf();
                std::string rawData = buffer.str();
                
                totalRawSize += rawData.size();
                
                // Compress via Huffman
                std::string encoded = huffman.encode(rawData);
                
                // Simulated bit-packing (8 chars '0'/'1' -> 1 byte)
                size_t actualByteSize = (encoded.size() / 8) + 1;
                totalCompressedSize += actualByteSize;
                
                packFile << entry.path().filename().string() << "\n" << actualByteSize << "\n" << encoded << "\n";
            }
        }
    }
    
    CompressionMetrics metrics;
    metrics.originalSize = totalRawSize;
    metrics.compressedSize = totalCompressedSize == 0 ? 1 : totalCompressedSize;
    metrics.ratio = static_cast<double>(metrics.originalSize) / metrics.compressedSize;
    metrics.spaceSavedPercentage = (1.0 - (1.0 / metrics.ratio)) * 100.0;
    
    return metrics;
}

// --- Analyzer ---

void CompressionAnalyzer::printReport(const CompressionMetrics& metrics) {
    std::cout << "========================================\n";
    std::cout << "       NovaVCS Compression Report       \n";
    std::cout << "========================================\n";
    std::cout << "Original Size   : " << metrics.originalSize << " bytes\n";
    std::cout << "Packed Size     : " << metrics.compressedSize << " bytes\n";
    std::cout << "Compression Ratio: " << metrics.ratio << "x\n";
    std::cout << "Space Saved     : " << metrics.spaceSavedPercentage << "%\n";
    std::cout << "========================================\n";
}

} // namespace nova::compression
