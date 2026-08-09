#pragma once
#include "nova/core/commit.hpp"
#include "nova/storage/object_db.hpp"
#include <filesystem>
#include <optional>

namespace nova::core {

class CommitGraph {
public:
    CommitGraph(const std::filesystem::path& repoRoot);

    // Core operations
    std::string createCommit(const std::string& treeOid, const std::string& message, const std::string& author);
    std::string amendCommit(const std::string& newTreeOid, const std::string& newMessage);
    
    // Traversal and Search
    std::vector<Commit> getHistory();
    std::vector<Commit> searchCommits(const std::string& query);

private:
    std::filesystem::path repoRoot_;
    nova::storage::ObjectDB db_;

    std::optional<std::string> getHeadOid() const;
    void updateHead(const std::string& commitOid);
    Commit loadCommit(const std::string& oid);
};

} // namespace nova::core
