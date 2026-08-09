#include "nova/core/commit_graph.hpp"
#include "nova/crypto/sha256.hpp"
#include <fstream>
#include <queue>
#include <unordered_set>
#include <iostream>

namespace nova::core {

CommitGraph::CommitGraph(const std::filesystem::path& repoRoot) 
    : repoRoot_(repoRoot), db_(repoRoot) {}

std::optional<std::string> CommitGraph::getHeadOid() const {
    std::ifstream headFile(repoRoot_ / ".nova" / "HEAD");
    if (!headFile.is_open()) return std::nullopt;
    
    std::string content;
    std::getline(headFile, content);
    if (content.empty()) return std::nullopt;
    
    // If it's a symbolic reference (like "ref: refs/heads/main")
    if (content.find("ref: ") == 0) {
        std::string refPath = content.substr(5);
        std::ifstream refFile(repoRoot_ / ".nova" / refPath);
        if (!refFile.is_open()) {
            return std::nullopt; // Branch file doesn't exist yet (this is the first commit)
        }
        std::string oid;
        refFile >> oid;
        return oid.empty() ? std::nullopt : std::make_optional(oid);
    }
    
    // Otherwise, it's a direct OID (detached HEAD state)
    return content;
}

void CommitGraph::updateHead(const std::string& commitOid) {
    std::ifstream headIn(repoRoot_ / ".nova" / "HEAD");
    std::string content;
    if (headIn.is_open()) std::getline(headIn, content);
    headIn.close();
    
    if (content.find("ref: ") == 0) {
        // Update the branch file (e.g., refs/heads/main)
        std::string refPath = content.substr(5);
        std::filesystem::path fullRefPath = repoRoot_ / ".nova" / refPath;
        std::filesystem::create_directories(fullRefPath.parent_path());
        std::ofstream refFile(fullRefPath, std::ios::trunc);
        refFile << commitOid;
    } else {
        // Update HEAD directly (detached state)
        std::ofstream headOut(repoRoot_ / ".nova" / "HEAD", std::ios::trunc);
        headOut << commitOid;
    }
}

Commit CommitGraph::loadCommit(const std::string& oid) {
    // 1. Unpack the pair returned by ObjectDB
    std::pair<std::string, std::string> obj = db_.readObject(oid);
    
    // obj.first is the type (e.g., "commit")
    // obj.second is the actual data payload
    std::string rawData = obj.second; 

    // 2. Strip the header if ObjectDB left it attached
    size_t nullPos = rawData.find('\0');
    if (nullPos != std::string::npos) {
        rawData = rawData.substr(nullPos + 1);
    }
    
    // 3. Deserialize
    Commit c = Commit::deserialize(rawData);
    c.oid = oid;
    return c;
}

std::string CommitGraph::createCommit(const std::string& treeOid, const std::string& message, const std::string& author) {
    Commit commit;
    commit.tree_oid = treeOid;
    commit.message = message;
    commit.author = author;
    commit.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch()).count();

    if (auto parent = getHeadOid()) {
        commit.parents.push_back(*parent);
    }

    if (!commit.isValid()) throw std::runtime_error("Invalid commit structure.");

    std::string serialized = commit.serialize();
    std::string oid = db_.writeObject("commit", serialized);
    updateHead(oid);
    
    return oid;
}

std::string CommitGraph::amendCommit(const std::string& newTreeOid, const std::string& newMessage) {
    auto headOid = getHeadOid();
    if (!headOid) throw std::runtime_error("No commit to amend.");

    Commit oldCommit = loadCommit(*headOid);
    
    Commit amended;
    amended.tree_oid = newTreeOid.empty() ? oldCommit.tree_oid : newTreeOid;
    amended.message = newMessage.empty() ? oldCommit.message : newMessage;
    amended.author = oldCommit.author;
    amended.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();
    amended.parents = oldCommit.parents; // Inherit parents, replacing the old commit in the DAG

    std::string serialized = amended.serialize();
    std::string newOid = db_.writeObject("commit", serialized);
    updateHead(newOid);

    return newOid;
}

std::vector<Commit> CommitGraph::getHistory() {
    std::vector<Commit> history;
    auto headOid = getHeadOid();
    if (!headOid) return history;

    // BFS Traversal of the DAG
    std::queue<std::string> queue;
    std::unordered_set<std::string> visited;

    queue.push(*headOid);
    visited.insert(*headOid);

    while (!queue.empty()) {
        std::string currentOid = queue.front();
        queue.pop();

        Commit c = loadCommit(currentOid);
        history.push_back(c);

        for (const auto& parentOid : c.parents) {
            if (visited.find(parentOid) == visited.end()) {
                visited.insert(parentOid);
                queue.push(parentOid);
            }
        }
    }
    return history;
}

std::vector<Commit> CommitGraph::searchCommits(const std::string& query) {
    std::vector<Commit> results;
    auto history = getHistory();
    for (const auto& commit : history) {
        if (commit.message.find(query) != std::string::npos || 
            commit.author.find(query) != std::string::npos) {
            results.push_back(commit);
        }
    }
    return results;
}

} // namespace nova::core
