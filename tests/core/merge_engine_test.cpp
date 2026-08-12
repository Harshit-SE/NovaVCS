#include <gtest/gtest.h>
#include "nova/core/merge_engine.hpp"

using namespace nova::merge;

TEST(MergeEngineTest, CleanThreeWayMerge) {
    std::string base = "line1\nline2\nline3";
    std::string ours = "line1\nline2_modified\nline3";
    std::string theirs = "line1\nline2\nline3_modified";
    
    MergeResult result = MergeEngine::threeWayMerge(base, ours, theirs);
    
    EXPECT_TRUE(result.isClean);
    EXPECT_EQ(result.conflicts.size(), 0);
    
    std::string expected = "line1\nline2_modified\nline3_modified\n";
    EXPECT_EQ(result.mergedContent, expected);
}

TEST(MergeEngineTest, ConflictDetection) {
    std::string base = "int port = 8080;";
    std::string ours = "int port = 3000;";
    std::string theirs = "int port = 5432;";
    
    MergeResult result = MergeEngine::threeWayMerge(base, ours, theirs);
    
    EXPECT_FALSE(result.isClean);
    ASSERT_EQ(result.conflicts.size(), 1);
    
    EXPECT_EQ(result.conflicts[0].ourContent, "int port = 3000;");
    EXPECT_EQ(result.conflicts[0].theirContent, "int port = 5432;");
}

TEST(MergeEngineTest, AIContextGeneration) {
    std::string base = "db_engine: sqlite";
    std::string ours = "db_engine: in_memory";
    std::string theirs = "db_engine: postgres";
    
    MergeResult result = MergeEngine::threeWayMerge(base, ours, theirs);
    ASSERT_FALSE(result.isClean);
    
    std::string payload = result.conflicts[0].generateAIContext();
    EXPECT_TRUE(payload.find("\"ours\": \"db_engine: in_memory\"") != std::string::npos);
    EXPECT_TRUE(payload.find("\"theirs\": \"db_engine: postgres\"") != std::string::npos);
}
