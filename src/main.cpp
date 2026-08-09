#include "nova/cli/parser.hpp"
#include "nova/core/repository.hpp" 
#include "nova/storage/object_db.hpp" 
#include "nova/core/logger.hpp"
#include "nova/crypto/sha256.hpp"
#include "nova/core/commit_graph.hpp"
#include "nova/core/branch_manager.hpp"
#include "IndexManager.hpp" 
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
   // Phase 2: add
    cli.registerCommand("add", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            nova::core::Logger::error("Missing filename. Usage: nova add <file>");
            return 1;
        }
        std::string filename = args[0];
        try {
            auto repoOpt = nova::core::Repository::discover(std::filesystem::current_path());
            if (!repoOpt) {
                nova::core::Logger::error("fatal: not a nova repository");
                return 1;
            }
            std::filesystem::path repoRoot = repoOpt->getRootPath();
            std::string indexPath = (repoRoot / ".nova" / "index").string();
            
            // Optional: Still write to CAS ObjectDB for blob persistence
            std::ifstream file(filename, std::ios::binary);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                nova::storage::ObjectDB db(repoRoot);
                db.writeObject("blob", buffer.str());
            }
            
            // FIX: Call addFile with only the filename argument
            IndexManager index(repoRoot.string());
            index.loadIndex(indexPath);
            index.addFile(filename);
            index.saveIndex(indexPath);
            
            std::cout << "[SUCCESS] Added '" << filename << "' to the staging area.\n";
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(std::string("Failed to add file: ") + e.what());
            return 1;
        }
    }, "Add file contents to the staging area");

    // Phase 3: status
    cli.registerCommand("status", [](const std::vector<std::string>& args) {
        try {
            auto repoOpt = nova::core::Repository::discover(std::filesystem::current_path());
            if (!repoOpt) {
                nova::core::Logger::error("fatal: not a nova repository (or any of the parent directories): .nova");
                return 1;
            }
            std::filesystem::path repoRoot = repoOpt->getRootPath();
            std::string indexPath = (repoRoot / ".nova" / "index").string();
            
            IndexManager index(repoRoot.string());
            index.loadIndex(indexPath);
            
            auto status = index.generateStatus();
            
            std::cout << "NovaVCS Status:\n";
            
            if (!status.renamed.empty()) {
                std::cout << "\nRenamed files:\n";
                for (const auto& [old_path, new_path] : status.renamed) {
                    std::cout << "  (renamed)    " << old_path << " -> " << new_path << "\n";
                }
            }
            if (!status.deleted.empty()) {
                std::cout << "\nDeleted files:\n";
                for (const auto& file : status.deleted) {
                    std::cout << "  (deleted)    " << file << "\n";
                }
            }
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
            
            if (status.untracked.empty() && status.modified.empty() && status.staged.empty() && status.deleted.empty() && status.renamed.empty()) {
                std::cout << "Working tree clean.\n";
            }
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Show the working tree status");

    // Phase 4: commit
    cli.registerCommand("commit", [](const std::vector<std::string>& args) {
        bool amend = false;
        std::string message;

        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--amend") {
                amend = true;
            } else if (args[i] == "-m" && i + 1 < args.size()) {
                message = args[++i];
            }
        }

        if (message.empty()) {
            nova::core::Logger::error("Usage: nova commit [--amend] -m \"Message\"");
            return 1;
        }
        
        try {
            auto repoOpt = nova::core::Repository::discover(std::filesystem::current_path());
            if (!repoOpt) throw std::runtime_error("Not a nova repository.");
            
            std::string dummyTreeOid = nova::crypto::SHA256::hash("dummy_tree_state");
            
            nova::core::CommitGraph graph(repoOpt->getRootPath());
            
            std::string oid;
            if (amend) {
                oid = graph.amendCommit(dummyTreeOid, message);
            } else {
                oid = graph.createCommit(dummyTreeOid, message, "hetronom.live");
            }
            
            std::cout << "[SUCCESS] " << (amend ? "Amended" : "Created") << " commit " << oid.substr(0, 7) << "\n";
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Record changes to the repository");

    // Phase 4: log
    cli.registerCommand("log", [](const std::vector<std::string>& args) {
        try {
            auto repoOpt = nova::core::Repository::discover(std::filesystem::current_path());
            if (!repoOpt) throw std::runtime_error("Not a nova repository.");
            
            nova::core::CommitGraph graph(repoOpt->getRootPath());
            auto history = graph.getHistory();
            
            for (const auto& commit : history) {
                std::cout << "commit " << commit.oid << "\n";
                std::cout << "Author: " << commit.author << "\n";
                std::cout << "Date:   " << commit.timestamp << "\n\n";
                std::cout << "    " << commit.message << "\n\n";
            }
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Show commit logs");
    
    // Phase 5: branch
    cli.registerCommand("branch", [](const std::vector<std::string>& args) {
        try {
            auto repoOpt = nova::core::Repository::discover(std::filesystem::current_path());
            if (!repoOpt) throw std::runtime_error("Not a nova repository.");
            nova::core::BranchManager bm(repoOpt->getRootPath());

            if (args.empty()) {
                // List branches
                auto current = bm.getCurrentBranch();
                for (const auto& branch : bm.listBranches()) {
                    if (branch == current) std::cout << "* \033[32m" << branch << "\033[0m\n";
                    else std::cout << "  " << branch << "\n";
                }
            } else if (args[0] == "-d" && args.size() > 1) {
                // Delete branch
                bm.deleteBranch(args[1]);
                std::cout << "Deleted branch " << args[1] << "\n";
            } else if (args[0] == "-m" && args.size() > 1) {
                // Rename current branch
                std::string current = bm.getCurrentBranch();
                if (current.empty()) throw std::runtime_error("Not currently on any branch.");
                bm.renameBranch(current, args[1]);
            } else {
                // Create branch
                bm.createBranch(args[0]);
            }
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "List, create, or delete branches");

    // Phase 5: checkout
    cli.registerCommand("checkout", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            nova::core::Logger::error("Usage: nova checkout <branch|commit>");
            return 1;
        }
        try {
            auto repoOpt = nova::core::Repository::discover(std::filesystem::current_path());
            if (!repoOpt) throw std::runtime_error("Not a nova repository.");
            nova::core::BranchManager bm(repoOpt->getRootPath());
            
            bm.checkout(args[0]);
            
            if (bm.isDetachedHead()) {
                std::cout << "Note: switching to '" << args[0] << "'.\nYou are in 'detached HEAD' state.\n";
            } else {
                std::cout << "Switched to branch '" << args[0] << "'\n";
            }
            return 0;
        } catch (const std::exception& e) {
            nova::core::Logger::error(e.what());
            return 1;
        }
    }, "Switch branches or restore working tree files");

    return cli.parseAndRun(argc, argv);
}
