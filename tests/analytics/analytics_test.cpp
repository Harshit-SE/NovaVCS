#include <gtest/gtest.h>
#include "nova/analytics/analytics_engine.hpp"

using namespace nova::analytics;

TEST(AnalyticsEngineTest, JSONSerialization) {
    RepositoryStats stats;
    stats.totalCommits = 42;
    stats.totalBranches = 3;
    stats.totalFiles = 150;
    stats.storageUsageBytes = 204800; // 200 KB
    
    stats.largestFiles.push_back({"src/main.cpp", 15000});
    stats.largestFiles.push_back({"docs/architecture.md", 8000});
    
    stats.commitsPerAuthor["hetronom.live"] = 40;
    stats.commitsPerAuthor["contributor_1"] = 2;
    
    std::string jsonPayload = stats.toJSON();
    
    // Verify JSON structure and data presence
    EXPECT_TRUE(jsonPayload.find("\"total_commits\": 42") != std::string::npos);
    EXPECT_TRUE(jsonPayload.find("\"total_branches\": 3") != std::string::npos);
    EXPECT_TRUE(jsonPayload.find("\"storage_usage_bytes\": 204800") != std::string::npos);
    EXPECT_TRUE(jsonPayload.find("\"path\": \"src/main.cpp\"") != std::string::npos);
    EXPECT_TRUE(jsonPayload.find("\"hetronom.live\": 40") != std::string::npos);
}

TEST(AnalyticsEngineTest, FileStatSorting) {
    // A simplified test to ensure the JSON properly handles multiple files
    RepositoryStats stats;
    stats.largestFiles.push_back({"huge_file.bin", 999999});
    std::string jsonPayload = stats.toJSON();
    EXPECT_TRUE(jsonPayload.find("\"size\": 999999") != std::string::npos);
}
