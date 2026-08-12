#include "nova/core/merge_engine.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace nova::merge {

std::vector<std::string> MergeEngine::splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string Conflict::generateAIContext() const {
    // Generates a structured payload for external API/Agent ingestion
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"conflict_type\": \"three_way\",\n";
    ss << "  \"base\": \"" << baseContent << "\",\n";
    ss << "  \"ours\": \"" << ourContent << "\",\n";
    ss << "  \"theirs\": \"" << theirContent << "\"\n";
    ss << "}";
    return ss.str();
}

std::string MergeResult::getFormattedOutput() const {
    if (isClean) return mergedContent;
    
    std::ostringstream ss;
    // In a full implementation, this interweaves the text and conflict markers.
    // For this engine, we append the raw conflicts to the end of the clean text boundary.
    ss << mergedContent;
    for (const auto& conflict : conflicts) {
        ss << "<<<<<<< HEAD\n"
           << conflict.ourContent << "\n"
           << "=======\n"
           << conflict.theirContent << "\n"
           << ">>>>>>> TARGET\n";
    }
    return ss.str();
}

MergeResult MergeEngine::threeWayMerge(const std::string& baseText, 
                                       const std::string& ourText, 
                                       const std::string& theirText) {
    auto baseLines = splitLines(baseText);
    auto ourLines = splitLines(ourText);
    auto theirLines = splitLines(theirText);
    
    MergeResult result;
    result.isClean = true;
    std::ostringstream merged;
    
    size_t maxLines = std::max({baseLines.size(), ourLines.size(), theirLines.size()});
    
    for (size_t i = 0; i < maxLines; ++i) {
        std::string b = (i < baseLines.size()) ? baseLines[i] : "";
        std::string o = (i < ourLines.size()) ? ourLines[i] : "";
        std::string t = (i < theirLines.size()) ? theirLines[i] : "";
        
        if (o == t) {
            // Both made the same change or no change
            if (!o.empty() || i < ourLines.size()) merged << o << "\n";
        } else if (o == b && t != b) {
            // Only they changed it
            if (!t.empty() || i < theirLines.size()) merged << t << "\n";
        } else if (t == b && o != b) {
            // Only we changed it
            if (!o.empty() || i < ourLines.size()) merged << o << "\n";
        } else {
            // Both changed it differently -> CONFLICT
            result.isClean = false;
            Conflict c;
            c.lineStart = i;
            c.lineEnd = i;
            c.baseContent = b;
            c.ourContent = o;
            c.theirContent = t;
            result.conflicts.push_back(c);
        }
    }
    
    result.mergedContent = merged.str();
    return result;
}

void MergeEngine::previewMerge(const std::string& baseText, 
                               const std::string& ourText, 
                               const std::string& theirText) {
    auto result = threeWayMerge(baseText, ourText, theirText);
    if (result.isClean) {
        std::cout << "\033[32mMerge Preview: Clean. No conflicts detected.\033[0m\n";
    } else {
        std::cout << "\033[31mMerge Preview: " << result.conflicts.size() << " conflict(s) detected.\033[0m\n";
        for (const auto& c : result.conflicts) {
            std::cout << "Conflict at line " << c.lineStart + 1 << ":\n"
                      << c.generateAIContext() << "\n";
        }
    }
}

bool MergeEngine::mergeFiles(const fs::path& baseFile,
                             const fs::path& ourFile,
                             const fs::path& theirFile,
                             const fs::path& outputPath) {
    auto readFile = [](const fs::path& p) -> std::string {
        if (!fs::exists(p)) return "";
        std::ifstream file(p);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };

    auto result = threeWayMerge(readFile(baseFile), readFile(ourFile), readFile(theirFile));
    
    std::ofstream out(outputPath);
    out << result.getFormattedOutput();
    
    return result.isClean;
}

std::string MergeEngine::findLowestCommonAncestor(const std::string& commitA, const std::string& commitB) {
    // Placeholder for DAG traversal. In a full DAG, this uses BFS to find the first shared node.
    return "base_commit_hash"; 
}

} // namespace nova::merge
