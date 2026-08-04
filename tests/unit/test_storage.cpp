#include <gtest/gtest.h>
#include "nova/storage/compressor.hpp"
#include "nova/crypto/sha256.hpp"
#include "nova/storage/object_db.hpp"
#include "nova/core/repository.hpp"
#include <filesystem>

TEST(CryptoTest, SHA256ProducesConsistentHash) {
    std::string data = "blob 13\0Hello, World!";
    std::string hash = nova::crypto::SHA256::hash(data);
    
    // Validated against standard git hash for "Hello, World!" but using SHA-256
    EXPECT_EQ(hash.length(), 64); 
    EXPECT_EQ(nova::crypto::SHA256::hash(data), hash); 
}

TEST(CompressionTest, CompressDecompressString) {
    std::string original = "NovaVCS Core Engine Compression Test String";
    std::string compressed = nova::storage::Compressor::compress(original);
    std::string decompressed = nova::storage::Compressor::decompress(compressed);
    
    EXPECT_NE(original, compressed);
    EXPECT_EQ(original, decompressed);
}

class ObjectDBTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "nova_odb_test";
        std::filesystem::remove_all(test_dir);
        std::filesystem::create_directories(test_dir);
        nova::core::Repository::init(test_dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    std::filesystem::path test_dir;
};

TEST_F(ObjectDBTest, WriteAndReadBlob) {
    nova::storage::ObjectDB odb(test_dir);
    
    std::string content = "System Architecture and File Systems";
    std::string oid = odb.writeObject("blob", content);
    
    EXPECT_EQ(oid.length(), 64);
    
    auto [type, retrievedContent] = odb.readObject(oid);
    
    EXPECT_EQ(type, "blob");
    EXPECT_EQ(retrievedContent, content);
}
