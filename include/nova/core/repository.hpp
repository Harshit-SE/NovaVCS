/**
 * @file repository.hpp
 * @brief Core repository management
 */
#pragma once
#include <filesystem>
#include <optional>
#include <string>

namespace nova::core {

/**
 * @class Repository
 * @brief Represents a NovaVCS repository, handling initialization and discovery.
 */
class Repository {
public:
    static Repository init(const std::filesystem::path& path);
    static std::optional<Repository> discover(const std::filesystem::path& start_path);

    std::filesystem::path getNovaPath() const;
    std::filesystem::path getRootPath() const;
private:
    explicit Repository(std::filesystem::path path);
    std::filesystem::path repoRoot_;
};

} // namespace nova::core
