#include "nova/cli/parser.hpp"
#include "nova/core/repository.hpp" 
#include "nova/storage/object_db.hpp" 
#include "nova/core/logger.hpp"
#include "IndexManager.hpp" // <-- Phase 3
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
    nova::cli::Parser cli;

    // Phase 1: init
    cli.registerCommand("init", [](const std::vector<std::string>& args) {
        try {
            nova::core::Repository::init(std::filesystem::current_path());
            std::cout << "[SUCCESS] Initialized NovaVCS repository.\n";
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Initialize a new NovaVCS repository");

    // Phase 2: add
    cli.registerCommand("add", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            nova::core::Logger::error("Missing filename. Usage: nova add <file>");
            return 1;
        }
        std::string filename = args[0];
        try {
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open()) throw std::runtime_error("Could not open file: " + filename);
            std::stringstream buffer;
            buffer << file.rdbuf();
            
            nova::storage::ObjectDB db(std::filesystem::current_path());
            std::string hash = db.writeObject("blob", buffer.str());
            
            std::cout << "[SUCCESS] Added '" << filename << "' to the Object Database.\n";
            std::cout << "[INFO] Generated OID: " << hash << "\n";
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(std::string("Failed to add file: ") + e.what());
            return 1;
        }
    }, "Add file contents to the Object Database (CAS)");

    // Phase 3: status (The New Wiring)
    cli.registerCommand("status", [](const std::vector<std::string>& args) {
        try {
            IndexManager index(std::filesystem::current_path().string());
            auto status = index.generateStatus();
            
            std::cout << "NovaVCS Status:\n";
            
            if (!status.untracked.empty()) {
                std::cout << "\nUntracked files:\n";
                for (const auto& file : status.untracked) {
                    std::cout << "  (untracked)  " << file << "\n";
                }
            }
            if (!status.modified.empty()) {
                std::cout << "\nModified files:\n";
                for (const auto& file : status.modified) {
                    std::cout << "  (modified)   " << file << "\n";
                }
            }
            
            if (status.untracked.empty() && status.modified.empty() && status.staged.empty()) {
                std::cout << "Working tree clean.\n";
            }
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Show the working tree status");

    return cli.parseAndRun(argc, argv);
}
