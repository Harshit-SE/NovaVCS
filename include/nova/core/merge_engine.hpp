#ifndef NOVA_MERGE_ENGINE_HPP
#define NOVA_MERGE_ENGINE_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace nova::merge {

struct Conflict {
    int lineStart;
    int lineEnd;
    std::string baseContent;
    std::string ourContent;
    std::string theirContent;
    
    // Serializes conflict for external AI agent analysis
    std::string generateAIContext() const;
};

struct MergeResult {
    bool isClean;
    std::string mergedContent;
    std::vector<Conflict> conflicts;
    
    // Formats the output with standard <<<<<<< ======= >>>>>>> markers
    std::string getFormattedOutput() const;
};

class MergeEngine {
public:
    // Core Algorithms
    static MergeResult threeWayMerge(const std::string& baseText, 
                                     const std::string& ourText, 
                                     const std::string& theirText);

    // Repository Operations
    static bool mergeFiles(const std::filesystem::path& baseFile,
                           const std::filesystem::path& ourFile,
                           const std::filesystem::path& theirFile,
                           const std::filesystem::path& outputPath);

    static void previewMerge(const std::string& baseText, 
                             const std::string& ourText, 
                             const std::string& theirText);
                             
    static std::string findLowestCommonAncestor(const std::string& commitA, 
                                                const std::string& commitB);

private:
    static std::vector<std::string> splitLines(const std::string& text);
};

} // namespace nova::merge

#endif // NOVA_MERGE_ENGINE_HPP
