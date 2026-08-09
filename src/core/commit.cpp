#include "nova/core/commit.hpp"
#include <stdexcept>

namespace nova::core {

std::string Commit::serialize() const {
    std::ostringstream oss;
    oss << "tree " << tree_oid << "\n";
    for (const auto& p : parents) {
        oss << "parent " << p << "\n";
    }
    oss << "author " << author << " " << timestamp << "\n";
    oss << "\n" << message << "\n";
    return oss.str();
}

Commit Commit::deserialize(const std::string& data) {
    Commit commit;
    std::istringstream iss(data);
    std::string line;

    bool readingMessage = false;
    std::ostringstream msgStream;

    while (std::getline(iss, line)) {
        if (readingMessage) {
            msgStream << line << "\n";
            continue;
        }
        if (line.empty()) {
            readingMessage = true;
            continue;
        }

        std::istringstream lineStream(line);
        std::string key;
        lineStream >> key;

        if (key == "tree") {
            lineStream >> commit.tree_oid;
        } else if (key == "parent") {
            std::string parent;
            lineStream >> parent;
            commit.parents.push_back(parent);
        } else if (key == "author") {
            lineStream >> commit.author >> commit.timestamp;
        }
    }
    
    std::string fullMsg = msgStream.str();
    if (!fullMsg.empty() && fullMsg.back() == '\n') fullMsg.pop_back(); // strip trailing newline
    commit.message = fullMsg;

    return commit;
}

bool Commit::isValid() const {
    return !tree_oid.empty() && !author.empty() && !message.empty();
}

} // namespace nova::core
