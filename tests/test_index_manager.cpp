#include <gtest/gtest.h>
#include "IndexManager.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

class IndexManagerTest : public ::testing::Test {
protected:
    std::string testDir = "nova_test_repo";

    void SetUp() override {
        fs::create_directory(testDir);
    }

    void TearDown() override {
        fs::remove_all(testDir);
    }

    void createFile(const std::string& path, const std::string& content) {
        fs::path fullPath = fs::path(testDir) / path;
        fs::create_directories(fullPath.parent_path());
        std::ofstream file(fullPath);
        file << content;
        file.close();
    }
};

TEST_F(IndexManagerTest, AddAndRetrieveMetadata) {
    createFile("main.cpp", "int main() { return 0; }");
    IndexManager index(testDir);
    
    index.addFile("main.cpp", "dummy_hash_123");
    auto metadata = index.getMetadata("main.cpp");
    
    ASSERT_TRUE(metadata.has_value());
    EXPECT_EQ(metadata->path, "main.cpp");
    EXPECT_EQ(metadata->hash, "dummy_hash_123");
    EXPECT_TRUE(metadata->is_staged);
}

TEST_F(IndexManagerTest, GenerateStatusUntrackedAndModified) {
    createFile("app.cpp", "code");
    createFile("config.json", "{}");

    IndexManager index(testDir);
    index.addFile("app.cpp", "hash1");

    // Modify file to trigger timestamp/size change
    createFile("app.cpp", "modified code");

    RepoStatus status = index.generateStatus();
    
    EXPECT_EQ(status.untracked.size(), 1);
    EXPECT_EQ(status.untracked[0], "config.json");
    
    EXPECT_EQ(status.modified.size(), 1);
    EXPECT_EQ(status.modified[0], "app.cpp");
}

TEST_F(IndexManagerTest, RenameDetection) {
    createFile("old_name.txt", "same content");
    IndexManager index(testDir);
    
    // In our dummy hash logic, hash depends on file size. 
    // "same content" is 12 bytes.
    index.addFile("old_name.txt", "hash_12"); 

    // Simulate rename: delete old, create new with same content
    fs::remove(fs::path(testDir) / "old_name.txt");
    createFile("new_name.txt", "same content");

    RepoStatus status = index.generateStatus();

    EXPECT_EQ(status.renamed.size(), 1);
    EXPECT_EQ(status.renamed[0].first, "old_name.txt");
    EXPECT_EQ(status.renamed[0].second, "new_name.txt");
    EXPECT_EQ(status.deleted.size(), 0);
    EXPECT_EQ(status.untracked.size(), 0);
}

TEST_F(IndexManagerTest, IgnoreRules) {
    createFile(".novaignore", "*.log\nbuild/");
    createFile("test.log", "error");
    createFile("build/output.o", "binary");
    createFile("src/main.cpp", "code");

    IndexManager index(testDir);
    RepoStatus status = index.generateStatus();

    // Only main.cpp and .novaignore should be untracked. The others are ignored.
    EXPECT_EQ(status.untracked.size(), 2);
}
