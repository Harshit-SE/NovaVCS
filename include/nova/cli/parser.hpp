/**
 * @file parser.hpp
 * @brief Command Line Interface Parser
 */
#pragma once
#include <vector>
#include <string>
#include <functional>
#include <map>

namespace nova::cli {

/**
 * @class Parser
 * @brief Maps command line arguments to core actions.
 */
class Parser {
public:
    using CommandHandler = std::function<int(const std::vector<std::string>&)>;
    
    void registerCommand(const std::string& name, CommandHandler handler, const std::string& helpText);
    int parseAndRun(int argc, char** argv);
private:
    std::map<std::string, std::pair<CommandHandler, std::string>> commands_;
    void printHelp() const;
};

} // namespace nova::cli
