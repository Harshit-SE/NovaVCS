#include "nova/utils/fs_utils.hpp"
#include "nova/core/error.hpp"
#include <fstream>
#include <sstream>

namespace nova::utils {

void FSUtils::createDirectory(const fs::path& path) {
    if (!fs::exists(path)) {
        if (!fs::create_directories(path)) {
            throw core::NovaException("Failed to create directory: " + path.string());
        }
    }
}

void FSUtils::createFile(const fs::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw core::NovaException("Failed to create file: " + path.string());
    }
    file.write(content.c_str(), content.size());
}

bool FSUtils::exists(const fs::path& path) {
    return fs::exists(path);
}

std::string FSUtils::readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw core::NovaException("Failed to read file: " + path.string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}
