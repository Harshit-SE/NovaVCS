#ifndef NOVA_COMPRESSION_ENGINE_HPP
#define NOVA_COMPRESSION_ENGINE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace nova::compression {

struct CompressionMetrics {
    size_t originalSize;
    size_t compressedSize;
    double ratio; // Calculated as originalSize / compressedSize
    double spaceSavedPercentage;
};

class HuffmanCoder {
private:
    struct Node {
        char data;
        unsigned freq;
        std::unique_ptr<Node> left, right;
        Node(char d, unsigned f) : data(d), freq(f) {}
    };

    struct Compare {
        bool operator()(const Node* l, const Node* r) {
            return l->freq > r->freq;
        }
    };

    std::unordered_map<char, std::string> huffmanCodes;
    void generateCodes(const Node* root, const std::string& str);

public:
    std::string encode(const std::string& text);
    std::string decode(const std::string& encodedText, const Node* root);
};

class DeltaCompressor {
public:
    // Computes a delta between base and target using a simplified block-matching algorithm
    static std::string computeDelta(const std::string& base, const std::string& target);
    
    // Applies a delta to a base string to reconstruct the target
    static std::string applyDelta(const std::string& base, const std::string& delta);
};

class PackManager {
public:
    // Compresses loose objects in the repository into a single .pack file
    static CompressionMetrics packObjects(const std::filesystem::path& repoRoot);
    
    // Unpacks a .pack file back into loose objects (for extraction/recovery)
    static void unpackObjects(const std::filesystem::path& packFilePath, const std::filesystem::path& outputDir);
};

class CompressionAnalyzer {
public:
    static void printReport(const CompressionMetrics& metrics);
};

} // namespace nova::compression

#endif // NOVA_COMPRESSION_ENGINE_HPP
