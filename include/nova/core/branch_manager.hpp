#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace nova::core {

class BranchManager {
public:
    explicit BranchManager(const std::filesystem::path& repoRoot);

    // Branch Operations
    void createBranch(const std::string& name, const std::string& startPoint = "HEAD");
    void deleteBranch(const std::string& name);
    void renameBranch(const std::string& oldName, const std::string& newName);
    std::vector<std::string> listBranches() const;
    
    // HEAD Management
    void checkout(const std::string& target);
    std::string getCurrentBranch() const; 
    bool isDetachedHead() const;
    std::optional<std::string> getHeadOid() const;

    // Tag Operations
    void createTag(const std::string& name, const std::string& target = "HEAD");
    std::vector<std::string> listTags() const;

private:
    std::filesystem::path repoRoot_;
    
    std::string resolveTarget(const std::string& target) const;
    void writeRef(const std::filesystem::path& refPath, const std::string& oid);
    std::string readRef(const std::filesystem::path& refPath) const;
};

} // namespace nova::core
