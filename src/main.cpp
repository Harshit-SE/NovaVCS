#include "nova/cli/parser.hpp"
#include "nova/core/repository.hpp" 
#include "nova/storage/object_db.hpp" 
#include "nova/core/logger.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
    nova::cli::Parser cli;

    // Register 'init'
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

    // Register 'add'
    cli.registerCommand("add", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            nova::core::Logger::error("Missing filename. Usage: nova add <file>");
            return 1;
        }
        
        std::string filename = args[0];
        
        try {
            // 1. Read the raw content from the file
            std::ifstream file(filename, std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open file: " + filename);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            // 2. Instantiate your Object Database
            nova::storage::ObjectDB db(std::filesystem::current_path());
            
            // 3. Write the content as a "blob" using your method
            std::string hash = db.writeObject("blob", content);
            
            std::cout << "[SUCCESS] Added '" << filename << "' to the Object Database.\n";
            std::cout << "[INFO] Generated OID: " << hash << "\n";
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(std::string("Failed to add file: ") + e.what());
            return 1;
        }
    }, "Add file contents to the Object Database (CAS)");

    // Run the parser
    return cli.parseAndRun(argc, argv);
}
