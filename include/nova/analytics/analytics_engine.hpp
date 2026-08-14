#ifndef NOVA_ANALYTICS_ENGINE_HPP
#define NOVA_ANALYTICS_ENGINE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace nova::analytics {

struct FileStat {
    std::string path;
    size_t sizeBytes;
};

struct RepositoryStats {
    size_t totalCommits = 0;
    size_t totalBranches = 0;
    size_t totalFiles = 0;
    size_t storageUsageBytes = 0;
    
    std::vector<FileStat> largestFiles;
    std::unordered_map<std::string, int> commitsPerAuthor;
    std::unordered_map<std::string, int> commitsPerDay; 
    
    // Serializes the entire stats object to a JSON string for API consumption
    std::string toJSON() const;
};

class AnalyticsEngine {
public:
    // Main entry point to gather all repository metrics
    static RepositoryStats analyzeRepository(const std::filesystem::path& repoRoot);

private:
    static void analyzeStorage(const std::filesystem::path& repoRoot, RepositoryStats& stats);
    static void analyzeWorkingTree(const std::filesystem::path& repoRoot, RepositoryStats& stats);
    static void analyzeHistoryAndAuthors(const std::filesystem::path& repoRoot, RepositoryStats& stats);
};

} // namespace nova::analytics

#endif // NOVA_ANALYTICS_ENGINE_HPP
