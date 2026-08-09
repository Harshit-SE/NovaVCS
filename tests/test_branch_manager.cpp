#include <gtest/gtest.h>
#include "nova/core/branch_manager.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class BranchManagerTest : public ::testing::Test {
protected:
    fs::path testRepo = "test_repo_branches";
    
    void SetUp() override {
        fs::create_directories(testRepo / ".nova" / "refs" / "heads");
        // Simulate an initial commit on main
        std::ofstream headOut(testRepo / ".nova" / "HEAD");
        headOut << "ref: refs/heads/main";
        
        std::ofstream mainOut(testRepo / ".nova" / "refs" / "heads" / "main");
        mainOut << "dummy_commit_oid_123";
    }

    void TearDown() override {
        fs::remove_all(testRepo);
    }
};

TEST_F(BranchManagerTest, CreateAndListBranches) {
    nova::core::BranchManager bm(testRepo);
    bm.createBranch("feature-auth");
    
    auto branches = bm.listBranches();
    EXPECT_EQ(branches.size(), 2);
    EXPECT_TRUE(std::find(branches.begin(), branches.end(), "main") != branches.end());
    EXPECT_TRUE(std::find(branches.begin(), branches.end(), "feature-auth") != branches.end());
}

TEST_F(BranchManagerTest, CheckoutUpdatesHEAD) {
    nova::core::BranchManager bm(testRepo);
    bm.createBranch("develop");
    bm.checkout("develop");
    
    EXPECT_EQ(bm.getCurrentBranch(), "develop");
    EXPECT_FALSE(bm.isDetachedHead());
}

TEST_F(BranchManagerTest, DetachedHeadCheckout) {
    nova::core::BranchManager bm(testRepo);
    bm.checkout("dummy_commit_oid_123");
    
    EXPECT_TRUE(bm.isDetachedHead());
    EXPECT_EQ(bm.getCurrentBranch(), "");
}

TEST_F(BranchManagerTest, DeleteBranchProtection) {
    nova::core::BranchManager bm(testRepo);
    bm.createBranch("feature");
    
    // Should successfully delete
    EXPECT_NO_THROW(bm.deleteBranch("feature"));
    
    // Should fail to delete current branch
    EXPECT_THROW(bm.deleteBranch("main"), std::runtime_error);
}
