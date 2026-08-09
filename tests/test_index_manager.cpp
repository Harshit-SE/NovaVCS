#include <gtest/gtest.h>
#include "IndexManager.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class IndexManagerTest : public ::testing::Test {
protected:
    fs::path testRepo = "test_repo_index";

    void SetUp() override {
        fs::create_directories(testRepo / ".nova");
    }

    void TearDown() override {
        fs::remove_all(testRepo);
    }
};

TEST_F(IndexManagerTest, AddAndRetrieveMetadata) {
    fs::path filePath = testRepo / "file.txt";
    {
        std::ofstream f(filePath);
        f << "Test file content for hashing.";
    }

    IndexManager index(testRepo.string());
    index.addFile("file.txt");

    auto metadata = index.getMetadata("file.txt");
    ASSERT_TRUE(metadata.has_value());
    EXPECT_EQ(metadata->path, "file.txt");
    EXPECT_FALSE(metadata->hash.empty());
    EXPECT_TRUE(metadata->is_staged);
}

TEST_F(IndexManagerTest, IgnoreRules) {
    {
        std::ofstream f(testRepo / ".novaignore");
        f << "*.log\nbuild/\n";
    }

    {
        std::ofstream(testRepo / "main.cpp");
        std::ofstream(testRepo / "debug.log");
        fs::create_directories(testRepo / "build");
        std::ofstream(testRepo / "build" / "output.o");
        std::ofstream(testRepo / "untracked.txt");
    }

    IndexManager index(testRepo.string());
    RepoStatus status = index.generateStatus();

    EXPECT_EQ(status.untracked.size(), 2);
}

TEST_F(IndexManagerTest, RenameDetection) {
    fs::path oldPath = testRepo / "old_name.txt";
    {
        std::ofstream f(oldPath);
        f << "Content that stays identical for rename detection.";
    }

    IndexManager index(testRepo.string());
    index.addFile("old_name.txt");
    index.saveIndex((testRepo / ".nova" / "index").string());

    fs::remove(oldPath);
    
    fs::path newPath = testRepo / "new_name.txt";
    {
        std::ofstream f(newPath);
        f << "Content that stays identical for rename detection.";
    }

    IndexManager index2(testRepo.string());
    index2.loadIndex((testRepo / ".nova" / "index").string());

    RepoStatus status = index2.generateStatus();
    
    ASSERT_EQ(status.renamed.size(), 1);
    EXPECT_EQ(status.renamed[0].first, "old_name.txt");
    EXPECT_EQ(status.renamed[0].second, "new_name.txt");
}

TEST_F(IndexManagerTest, StagedFileStaysStagedAfterSaveLoad) {
    fs::path filePath = testRepo / "main.cpp";
    {
        std::ofstream f(filePath);
        f << "int main() { return 0; }";
    }

    // Place the index file inside .nova/ so it is automatically ignored by IgnoreEngine
    std::string indexPath = (testRepo / ".nova" / "index_index.bin").string();

    {
        IndexManager index(testRepo.string());
        index.addFile("main.cpp");
        index.saveIndex(indexPath);
    }

    {
        IndexManager index2(testRepo.string());
        index2.loadIndex(indexPath);
        RepoStatus status = index2.generateStatus();
        EXPECT_EQ(status.staged.size(), 1);
        EXPECT_TRUE(status.modified.empty());
        EXPECT_TRUE(status.untracked.empty());
    }
}
