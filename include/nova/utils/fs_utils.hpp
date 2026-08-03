/**
 * @file fs_utils.hpp
 * @brief Filesystem utility functions
 */
#pragma once
#include <filesystem>
#include <string>

namespace nova::utils {
namespace fs = std::filesystem;

class FSUtils {
public:
    static void createDirectory(const fs::path& path);
    static void createFile(const fs::path& path, const std::string& content = "");
    static bool exists(const fs::path& path);
    static std::string readFile(const fs::path& path);
};

} // namespace nova::utils
