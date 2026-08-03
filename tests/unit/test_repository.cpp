#include <gtest/gtest.h>
#include "nova/core/repository.hpp"
#include "nova/utils/fs_utils.hpp"
#include "nova/core/error.hpp"
#include <filesystem>

class RepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "nova_test_repo";
        std::filesystem::remove_all(test_dir);
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    std::filesystem::path test_dir;
};

TEST_F(RepositoryTest, InitializationCreatesCorrectStructure) {
    auto repo = nova::core::Repository::init(test_dir);
    
    auto nova_dir = test_dir / ".nova";
    EXPECT_TRUE(nova::utils::FSUtils::exists(nova_dir));
    EXPECT_TRUE(nova::utils::FSUtils::exists(nova_dir / "objects"));
    EXPECT_TRUE(nova::utils::FSUtils::exists(nova_dir / "refs" / "heads"));
    EXPECT_TRUE(nova::utils::FSUtils::exists(nova_dir / "HEAD"));
    EXPECT_TRUE(nova::utils::FSUtils::exists(nova_dir / "config"));
    EXPECT_TRUE(nova::utils::FSUtils::exists(nova_dir / "index"));
}

TEST_F(RepositoryTest, FailsIfAlreadyInitialized) {
    nova::core::Repository::init(test_dir);
    EXPECT_THROW(nova::core::Repository::init(test_dir), nova::core::NovaException);
}

TEST_F(RepositoryTest, CanDiscoverRepository) {
    nova::core::Repository::init(test_dir);
    
    // Create a deep nested directory
    auto deep_dir = test_dir / "src" / "deep" / "nested";
    std::filesystem::create_directories(deep_dir);
    
    auto repo = nova::core::Repository::discover(deep_dir);
    ASSERT_TRUE(repo.has_value());
    EXPECT_EQ(repo->getRootPath(), test_dir);
}
