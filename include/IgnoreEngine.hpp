#ifndef NOVA_IGNORE_ENGINE_HPP
#define NOVA_IGNORE_ENGINE_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

class IgnoreEngine {
public:
    IgnoreEngine() {
        // Default ignore rules (e.g., the NovaVCS directory itself)
        rules.push_back(".nova");
    }

    void loadRules(const std::string& ignoreFilePath) {
        std::ifstream file(ignoreFilePath);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                rules.push_back(line);
            }
        }
    }

    bool isIgnored(const std::string& path) const {
        for (const auto& rule : rules) {
            if (path.find(rule) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

private:
    std::vector<std::string> rules;
};

#endif // NOVA_IGNORE_ENGINE_HPP
