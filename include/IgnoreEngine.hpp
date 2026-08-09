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

    // Make sure this is at the top of the file!

// ... inside your IgnoreEngine class ...

bool isIgnored(const std::string& path) const {
    std::filesystem::path p(path);
    
    for (const auto& rule : rules) {
        // Iterate through each discrete folder or file component in the path
        for (const auto& component : p) {
            // Only trigger if an exact component name matches the ignore rule
            if (component.string() == rule) {
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
