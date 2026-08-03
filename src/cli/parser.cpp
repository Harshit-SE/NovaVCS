#include "nova/cli/parser.hpp"
#include "nova/core/logger.hpp"
#include <iostream>

namespace nova::cli {

void Parser::registerCommand(const std::string& name, CommandHandler handler, const std::string& helpText) {
    commands_[name] = {handler, helpText};
}

void Parser::printHelp() const {
    std::cout << "NovaVCS - Next-Gen Version Control System\n\nUsage: nova <command> [args...]\n\nCommands:\n";
    for (const auto& [name, data] : commands_) {
        std::cout << "  " << name << "\t" << data.second << "\n";
    }
}

int Parser::parseAndRun(int argc, char** argv) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string command = argv[1];
    if (command == "--help" || command == "-h") {
        printHelp();
        return 0;
    }

    auto it = commands_.find(command);
    if (it != commands_.end()) {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return it->second.first(args);
    } else {
        core::Logger::error("Unknown command: " + command);
        printHelp();
        return 1;
    }
}

}
