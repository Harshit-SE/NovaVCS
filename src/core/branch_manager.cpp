#include "nova/core/branch_manager.hpp"
#include <fstream>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

namespace nova::core {

BranchManager::BranchManager(const fs::path& repoRoot) : repoRoot_(repoRoot) {}

std::string BranchManager::readRef(const fs::path& refPath) const {
    std::ifstream file(refPath);
    if (!file.is_open()) return "";
    std::string oid;
    file >> oid;
    return oid;
}

void BranchManager::writeRef(const fs::path& refPath, const std::string& oid) {
    fs::create_directories(refPath.parent_path());
    std::ofstream file(refPath, std::ios::trunc);
    if (!file) throw std::runtime_error("Failed to write reference: " + refPath.string());
    file << oid;
}

std::optional<std::string> BranchManager::getHeadOid() const {
    std::ifstream headFile(repoRoot_ / ".nova" / "HEAD");
    if (!headFile.is_open()) return std::nullopt;
    
    std::string content;
    std::getline(headFile, content);
    if (content.empty()) return std::nullopt;
    
    if (content.find("ref: ") == 0) {
        std::string refPath = content.substr(5);
        std::string oid = readRef(repoRoot_ / ".nova" / refPath);
        return oid.empty() ? std::nullopt : std::make_optional(oid);
    }
    return content; // Detached HEAD (direct OID)
}

std::string BranchManager::resolveTarget(const std::string& target) const {
    if (target == "HEAD") {
        auto oid = getHeadOid();
        if (!oid) throw std::runtime_error("HEAD is unresolved (no commits yet).");
        return *oid;
    }
    
    // Check if it's a branch
    fs::path branchPath = repoRoot_ / ".nova" / "refs" / "heads" / target;
    if (fs::exists(branchPath)) {
        return readRef(branchPath);
    }

    // Check if it's a tag
    fs::path tagPath = repoRoot_ / ".nova" / "refs" / "tags" / target;
    if (fs::exists(tagPath)) {
        return readRef(tagPath);
    }

    // Assume it's already a raw OID
    return target; 
}

void BranchManager::createBranch(const std::string& name, const std::string& startPoint) {
    fs::path branchPath = repoRoot_ / ".nova" / "refs" / "heads" / name;
    if (fs::exists(branchPath)) {
        throw std::runtime_error("fatal: A branch named '" + name + "' already exists.");
    }
    std::string oid = resolveTarget(startPoint);
    writeRef(branchPath, oid);
}

void BranchManager::deleteBranch(const std::string& name) {
    if (getCurrentBranch() == name) {
        throw std::runtime_error("fatal: Cannot delete branch '" + name + "' checked out at HEAD");
    }
    fs::path branchPath = repoRoot_ / ".nova" / "refs" / "heads" / name;
    if (!fs::exists(branchPath)) {
        throw std::runtime_error("error: branch '" + name + "' not found.");
    }
    fs::remove(branchPath);
}

void BranchManager::renameBranch(const std::string& oldName, const std::string& newName) {
    fs::path oldPath = repoRoot_ / ".nova" / "refs" / "heads" / oldName;
    fs::path newPath = repoRoot_ / ".nova" / "refs" / "heads" / newName;
    
    if (!fs::exists(oldPath)) throw std::runtime_error("error: branch '" + oldName + "' not found.");
    if (fs::exists(newPath)) throw std::runtime_error("fatal: A branch named '" + newName + "' already exists.");
    
    fs::rename(oldPath, newPath);
    
    // Update HEAD if we are currently on the renamed branch
    if (getCurrentBranch() == oldName) {
        std::ofstream headOut(repoRoot_ / ".nova" / "HEAD", std::ios::trunc);
        headOut << "ref: refs/heads/" << newName;
    }
}

std::vector<std::string> BranchManager::listBranches() const {
    std::vector<std::string> branches;
    fs::path headsDir = repoRoot_ / ".nova" / "refs" / "heads";
    if (fs::exists(headsDir)) {
        for (const auto& entry : fs::directory_iterator(headsDir)) {
            branches.push_back(entry.path().filename().string());
        }
    }
    return branches;
}

std::string BranchManager::getCurrentBranch() const {
    std::ifstream headFile(repoRoot_ / ".nova" / "HEAD");
    if (!headFile.is_open()) return "";
    std::string content;
    std::getline(headFile, content);
    
    if (content.find("ref: refs/heads/") == 0) {
        return content.substr(16); // Extract branch name
    }
    return ""; // Detached
}

bool BranchManager::isDetachedHead() const {
    return getCurrentBranch().empty();
}

void BranchManager::checkout(const std::string& target) {
    fs::path branchPath = repoRoot_ / ".nova" / "refs" / "heads" / target;
    std::ofstream headOut(repoRoot_ / ".nova" / "HEAD", std::ios::trunc);
    
    if (fs::exists(branchPath)) {
        // Switch to branch (Attached HEAD)
        headOut << "ref: refs/heads/" << target;
    } else {
        // Switch to direct commit (Detached HEAD)
        std::string oid = resolveTarget(target);
        headOut << oid;
    }
    
    // Note: In Phase 6 (Diff/Checkout Engine), this is where we will 
    // update the working directory to match the target commit's tree.
}

void BranchManager::createTag(const std::string& name, const std::string& target) {
    fs::path tagPath = repoRoot_ / ".nova" / "refs" / "tags" / name;
    if (fs::exists(tagPath)) throw std::runtime_error("fatal: tag '" + name + "' already exists");
    std::string oid = resolveTarget(target);
    writeRef(tagPath, oid);
}

std::vector<std::string> BranchManager::listTags() const {
    std::vector<std::string> tags;
    fs::path tagsDir = repoRoot_ / ".nova" / "refs" / "tags";
    if (fs::exists(tagsDir)) {
        for (const auto& entry : fs::directory_iterator(tagsDir)) {
            tags.push_back(entry.path().filename().string());
        }
    }
    return tags;
}

} // namespace nova::core
