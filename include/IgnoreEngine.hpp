#ifndef NOVA_IGNORE_ENGINE_HPP
#define NOVA_IGNORE_ENGINE_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>

class IgnoreEngine {
public:
    IgnoreEngine() {
        rules.push_back(".nova");
        rules.push_back(".novaignore"); // Automatically ignore the config file itself
    }
    // ... rest of the class

    void loadRules(const std::string& ignoreFilePath) {
        std::ifstream file(ignoreFilePath);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty() && line[0] != '#') {
                rules.push_back(line);
            }
        }
    }

    bool matchGlob(const std::string& pattern, const std::string& text) const {
        if (pattern == text) return true;
        size_t firstStar = pattern.find('*');
        if (firstStar == std::string::npos) {
            return pattern == text;
        }
        std::string prefix = pattern.substr(0, firstStar);
        std::string suffix = pattern.substr(firstStar + 1);
        
        if (text.length() < prefix.length() + suffix.length()) return false;
        if (!prefix.empty() && text.substr(0, prefix.length()) != prefix) return false;
        if (!suffix.empty() && text.substr(text.length() - suffix.length()) != suffix) return false;
        return true;
    }

    bool isIgnored(const std::string& path) const {
        std::string normalizedPath = path;
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
        std::filesystem::path p(normalizedPath);
        std::string filename = p.filename().string();
        
        for (const auto& rule : rules) {
            std::string cleanRule = rule;
            if (!cleanRule.empty() && (cleanRule.back() == '/' || cleanRule.back() == '\\')) {
                cleanRule.pop_back();
            }

            if (cleanRule.empty()) continue;

            // 1. Exact path or filename match
            if (normalizedPath == cleanRule || filename == cleanRule) return true;

            // 2. Directory prefix match (e.g., "build/" matches "build/file.txt")
            if (normalizedPath.find(cleanRule + "/") == 0) return true;

            // 3. Glob matching on path or filename (e.g., "*.txt", "temp*")
            if (matchGlob(cleanRule, normalizedPath) || matchGlob(cleanRule, filename)) {
                return true;
            }

            // 4. Component match across directory hierarchy
            for (const auto& component : p) {
                if (component.string() == cleanRule || matchGlob(cleanRule, component.string())) {
                    return true;
                }
            }
        }
        return false;
    }

private:
    std::vector<std::string> rules;
};

#endif // NOVA_IGNORE_ENGINE_HPP
