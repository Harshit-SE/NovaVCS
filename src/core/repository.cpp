#include "nova/core/repository.hpp"
#include "nova/core/error.hpp"
#include "nova/core/logger.hpp"
#include "nova/utils/fs_utils.hpp"

namespace nova::core {

Repository::Repository(std::filesystem::path path) : repoRoot_(std::move(path)) {}

std::filesystem::path Repository::getNovaPath() const {
    return repoRoot_ / ".nova";
}

std::filesystem::path Repository::getRootPath() const {
    return repoRoot_;
}

Repository Repository::init(const std::filesystem::path& path) {
    auto novaDir = path / ".nova";
    if (utils::FSUtils::exists(novaDir)) {
        throw NovaException("Repository already exists at " + path.string());
    }

    // Creating hidden directory structure
    utils::FSUtils::createDirectory(novaDir);
    utils::FSUtils::createDirectory(novaDir / "objects");
    utils::FSUtils::createDirectory(novaDir / "objects" / "info");
    utils::FSUtils::createDirectory(novaDir / "objects" / "pack");
    utils::FSUtils::createDirectory(novaDir / "refs");
    utils::FSUtils::createDirectory(novaDir / "refs" / "heads");
    utils::FSUtils::createDirectory(novaDir / "refs" / "tags");

    // Repository configuration
    std::string configContent = "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = false\n";
    utils::FSUtils::createFile(novaDir / "config", configContent);

    // Initial HEAD pointer
    utils::FSUtils::createFile(novaDir / "HEAD", "ref: refs/heads/main\n");

    // Empty index file
    utils::FSUtils::createFile(novaDir / "index", "");

    Logger::info("Initialized empty NovaVCS repository in " + novaDir.string());
    return Repository(path);
}

std::optional<Repository> Repository::discover(const std::filesystem::path& start_path) {
    auto current = std::filesystem::absolute(start_path);
    while (true) {
        if (utils::FSUtils::exists(current / ".nova")) {
            return Repository(current);
        }
        if (current == current.parent_path()) {
            break; // Root reached
        }
        current = current.parent_path();
    }
    return std::nullopt;
}

}
