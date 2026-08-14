#include "nova/analytics/analytics_engine.hpp"
#include "nova/core/commit_graph.hpp"
#include "nova/core/branch_manager.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

namespace nova::analytics {

std::string RepositoryStats::toJSON() const {
    std::ostringstream json;
    json << "{\n";
    
    // Basic Stats
    json << "  \"total_commits\": " << totalCommits << ",\n";
    json << "  \"total_branches\": " << totalBranches << ",\n";
    json << "  \"total_files\": " << totalFiles << ",\n";
    json << "  \"storage_usage_bytes\": " << storageUsageBytes << ",\n";
    
    // Largest Files
    json << "  \"largest_files\": [\n";
    for (size_t i = 0; i < largestFiles.size(); ++i) {
        json << "    { \"path\": \"" << largestFiles[i].path << "\", \"size\": " << largestFiles[i].sizeBytes << " }";
        if (i < largestFiles.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";
    
    // File Ownership / Author Commits
    json << "  \"author_activity\": {\n";
    size_t authorCount = 0;
    for (const auto& [author, count] : commitsPerAuthor) {
        json << "    \"" << author << "\": " << count;
        if (++authorCount < commitsPerAuthor.size()) json << ",";
        json << "\n";
    }
    json << "  }\n";
    
    json << "}";
    return json.str();
}

RepositoryStats AnalyticsEngine::analyzeRepository(const fs::path& repoRoot) {
    RepositoryStats stats;
    analyzeStorage(repoRoot, stats);
    analyzeWorkingTree(repoRoot, stats);
    analyzeHistoryAndAuthors(repoRoot, stats);
    return stats;
}

void AnalyticsEngine::analyzeStorage(const fs::path& repoRoot, RepositoryStats& stats) {
    fs::path novaDir = repoRoot / ".nova";
    if (!fs::exists(novaDir)) return;
    
    for (const auto& entry : fs::recursive_directory_iterator(novaDir)) {
        if (entry.is_regular_file()) {
            stats.storageUsageBytes += entry.file_size();
        }
    }
}

void AnalyticsEngine::analyzeWorkingTree(const fs::path& repoRoot, RepositoryStats& stats) {
    std::vector<FileStat> allFiles;
    
    for (const auto& entry : fs::recursive_directory_iterator(repoRoot)) {
        std::string pathStr = entry.path().string();
        // Ignore the .nova directory and build artifacts for working tree stats
        if (pathStr.find(".nova") != std::string::npos || pathStr.find("build") != std::string::npos) {
            continue;
        }
        
        if (entry.is_regular_file()) {
            stats.totalFiles++;
            allFiles.push_back({fs::relative(entry.path(), repoRoot).string(), entry.file_size()});
        }
    }
    
    // Sort files by size descending
    std::sort(allFiles.begin(), allFiles.end(), [](const FileStat& a, const FileStat& b) {
        return a.sizeBytes > b.sizeBytes;
    });
    
    // Keep top 5 largest files
    for (size_t i = 0; i < std::min<size_t>(5, allFiles.size()); ++i) {
        stats.largestFiles.push_back(allFiles[i]);
    }
}

void AnalyticsEngine::analyzeHistoryAndAuthors(const fs::path& repoRoot, RepositoryStats& stats) {
    try {
        nova::core::CommitGraph graph(repoRoot);
        auto history = graph.getHistory(); // Assuming this returns a vector of commit objects
        
        stats.totalCommits = history.size();
        
        for (const auto& commit : history) {
            stats.commitsPerAuthor[commit.author]++;
            // In a full implementation, you would parse commit.timestamp to populate commitsPerDay for the timeline
        }
        
        nova::core::BranchManager bm(repoRoot);
        stats.totalBranches = bm.listBranches().size();
        
    } catch (...) {
        // Repo might not have any commits yet
        stats.totalCommits = 0;
        stats.totalBranches = 0;
    }
}

} // namespace nova::analytics
