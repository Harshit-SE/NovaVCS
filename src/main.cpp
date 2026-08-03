#include "nova/cli/parser.hpp"
#include "nova/core/repository.hpp"
#include "nova/core/logger.hpp"
#include "nova/core/error.hpp"
#include <filesystem>

int main(int argc, char** argv) {
    nova::cli::Parser parser;

    parser.registerCommand("init", [](const std::vector<std::string>& args) -> int {
        std::filesystem::path path = args.empty() ? std::filesystem::current_path() : std::filesystem::path(args[0]);
        try {
            nova::core::Repository::init(path);
            return 0;
        } catch (const nova::core::NovaException& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Initialize a new NovaVCS repository");

    return parser.parseAndRun(argc, argv);
}
