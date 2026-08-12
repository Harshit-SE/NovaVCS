#ifndef NOVA_DIFF_ENGINE_HPP
#define NOVA_DIFF_ENGINE_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace nova::diff {

enum class EditType {
    ADD,
    REMOVE,
    KEEP
};

struct Edit {
    EditType type;
    std::string text;
};

struct DiffResult {
    std::vector<Edit> edits;
    size_t additions = 0;
    size_t deletions = 0;
};

class DiffEngine {
private:
    // Precomputed hash structure for O(1) line comparisons
    struct HashedLine {
        size_t hash;
        std::string content;
        
        bool operator==(const HashedLine& other) const {
            return hash == other.hash && content == other.content;
        }
    };

    static std::vector<HashedLine> hashLines(const std::string& text);
    static std::vector<std::string> splitLines(const std::string& text);

public:
    // Core Algorithms
    static DiffResult computeMyersDiff(const std::string& oldText, const std::string& newText);
    static std::string computeLCS(const std::string& textA, const std::string& textB);

    // File & Directory Diffing
    static DiffResult diffFiles(const std::filesystem::path& oldFile, const std::filesystem::path& newFile);
    static void diffDirectories(const std::filesystem::path& oldDir, const std::filesystem::path& newDir);

    // Terminal Rendering
    static void printColoredDiff(const DiffResult& result);
    static void printSideBySideDiff(const DiffResult& result, int terminalWidth = 100);
};

} // namespace nova::diff

#endif // NOVA_DIFF_ENGINE_HPP
