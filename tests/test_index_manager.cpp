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

    // Exactly 2 files should be untracked (main.cpp and untracked.txt)
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
    
    // ASSERT_EQ prevents out-of-bounds segfaults if rename count is unexpected
    ASSERT_EQ(status.renamed.size(), 1);
    EXPECT_EQ(status.renamed[0].first, "old_name.txt");
    EXPECT_EQ(status.renamed[0].second, "new_name.txt");
}

TEST_F(IndexManagerTest, StagedFileStaysStagedAfterSaveLoad) {
    // Regression: saveIndex used to truncate mtime to whole seconds, so after
    // loadIndex the stored mtime no longer matched the file's real (sub-second)
    // mtime and generateStatus() reported the unmodified file as "modified".
    // This mirrors the real CLI flow: `nova add` saves the index and a later
    // `nova status` process loads it before scanning.
    createFile("main.cpp", "int main() { return 0; }");
    std::string indexPath = testDir + "_index.bin";  // sibling file, not scanned

    {
        IndexManager index(testDir);
        index.addFile("main.cpp", "hash1");  // staged; in-memory mtime is exact
        index.saveIndex(indexPath);
    }

    IndexManager reloaded(testDir);
    reloaded.loadIndex(indexPath);
    RepoStatus status = reloaded.generateStatus();

    fs::remove(indexPath);

    // The file was never touched, so it must come back as staged, not modified.
    EXPECT_EQ(status.modified.size(), 0u);
    ASSERT_EQ(status.staged.size(), 1u);
    EXPECT_EQ(status.staged[0], "main.cpp");
}
