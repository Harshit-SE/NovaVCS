#include "nova/core/diff_engine.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <iomanip>

namespace fs = std::filesystem;

namespace nova::diff {

std::vector<std::string> DiffEngine::splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::vector<DiffEngine::HashedLine> DiffEngine::hashLines(const std::string& text) {
    auto lines = splitLines(text);
    std::vector<HashedLine> hashedLines;
    hashedLines.reserve(lines.size());
    std::hash<std::string> hasher;
    
    for (auto& line : lines) {
        hashedLines.push_back({hasher(line), std::move(line)});
    }
    return hashedLines;
}

DiffResult DiffEngine::computeMyersDiff(const std::string& oldText, const std::string& newText) {
    auto A = hashLines(oldText);
    auto B = hashLines(newText);
    int N = A.size();
    int M = B.size();
    int MAX = N + M;
    
    if (MAX == 0) return {};

    std::vector<int> V(2 * MAX + 1);
    std::vector<std::vector<int>> trace;
    
    V[MAX + 1] = 0;
    int x, y;
    
    // Forward Myers O(ND) algorithm
    for (int D = 0; D <= MAX; D++) {
        std::vector<int> currentV = V;
        for (int k = -D; k <= D; k += 2) {
            if (k == -D || (k != D && V[MAX + k - 1] < V[MAX + k + 1])) {
                x = V[MAX + k + 1];
            } else {
                x = V[MAX + k - 1] + 1;
            }
            y = x - k;
            
            // Rolling Hash optimized comparison
            while (x < N && y < M && A[x] == B[y]) {
                x++; y++;
            }
            currentV[MAX + k] = x;
            if (x >= N && y >= M) {
                trace.push_back(currentV);
                goto backtrack;
            }
        }
        trace.push_back(currentV);
        V = currentV;
    }

backtrack:
    DiffResult result;
    x = N; y = M;
    for (int D = trace.size() - 1; D > 0; D--) {
        const auto& v = trace[D];
        int k = x - y;
        int prev_k;
        
        if (k == -D || (k != D && v[MAX + k - 1] < v[MAX + k + 1])) {
            prev_k = k + 1;
        } else {
            prev_k = k - 1;
        }
        
        int prev_x = trace[D - 1][MAX + prev_k];
        int prev_y = prev_x - prev_k;
        
        while (x > prev_x && y > prev_y) {
            result.edits.insert(result.edits.begin(), {EditType::KEEP, A[x-1].content});
            x--; y--;
        }
        if (x > prev_x) {
            result.edits.insert(result.edits.begin(), {EditType::REMOVE, A[prev_x].content});
            result.deletions++;
            x--;
        } else if (y > prev_y) {
            result.edits.insert(result.edits.begin(), {EditType::ADD, B[prev_y].content});
            result.additions++;
            y--;
        }
    }
    
    while (x > 0 && y > 0) {
        result.edits.insert(result.edits.begin(), {EditType::KEEP, A[x-1].content});
        x--; y--;
    }
    return result;
}

std::string DiffEngine::computeLCS(const std::string& textA, const std::string& textB) {
    auto diff = computeMyersDiff(textA, textB);
    std::string lcs;
    for (const auto& edit : diff.edits) {
        if (edit.type == EditType::KEEP) {
            lcs += edit.text + "\n";
        }
    }
    return lcs;
}

DiffResult DiffEngine::diffFiles(const fs::path& oldFile, const fs::path& newFile) {
    auto readFile = [](const fs::path& p) -> std::string {
        if (!fs::exists(p)) return "";
        std::ifstream file(p, std::ios::binary);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };
    return computeMyersDiff(readFile(oldFile), readFile(newFile));
}

void DiffEngine::diffDirectories(const fs::path& oldDir, const fs::path& newDir) {
    std::unordered_map<std::string, bool> processed;
    
    std::cout << "\n--- Directory Diff: " << oldDir.string() << " -> " << newDir.string() << " ---\n";
    
    for (const auto& entry : fs::recursive_directory_iterator(newDir)) {
        if (entry.is_directory()) continue;
        std::string relPath = fs::relative(entry.path(), newDir).string();
        fs::path oldFile = oldDir / relPath;
        
        if (!fs::exists(oldFile)) {
            std::cout << "\033[32mAdded: \033[0m" << relPath << "\n";
        } else {
            auto res = diffFiles(oldFile, entry.path());
            if (res.additions > 0 || res.deletions > 0) {
                std::cout << "\033[33mModified: \033[0m" << relPath 
                          << " (+" << res.additions << " -" << res.deletions << ")\n";
            }
        }
        processed[relPath] = true;
    }
    
    for (const auto& entry : fs::recursive_directory_iterator(oldDir)) {
        if (entry.is_directory()) continue;
        std::string relPath = fs::relative(entry.path(), oldDir).string();
        if (processed.find(relPath) == processed.end()) {
            std::cout << "\033[31mDeleted: \033[0m" << relPath << "\n";
        }
    }
}

void DiffEngine::printColoredDiff(const DiffResult& result) {
    for (const auto& edit : result.edits) {
        if (edit.type == EditType::ADD) {
            std::cout << "\033[32m+ " << edit.text << "\033[0m\n";
        } else if (edit.type == EditType::REMOVE) {
            std::cout << "\033[31m- " << edit.text << "\033[0m\n";
        } else {
            std::cout << "  " << edit.text << "\n";
        }
    }
}

void DiffEngine::printSideBySideDiff(const DiffResult& result, int terminalWidth) {
    int colWidth = (terminalWidth / 2) - 3;
    
    auto truncate = [colWidth](std::string s) {
        if (s.length() > static_cast<size_t>(colWidth)) return s.substr(0, colWidth - 3) + "...";
        return s + std::string(colWidth - s.length(), ' ');
    };

    std::cout << std::string(terminalWidth, '-') << "\n";
    for (const auto& edit : result.edits) {
        if (edit.type == EditType::KEEP) {
            std::cout << truncate(edit.text) << " | " << truncate(edit.text) << "\n";
        } else if (edit.type == EditType::ADD) {
            std::cout << std::string(colWidth, ' ') << " > \033[32m" << truncate(edit.text) << "\033[0m\n";
        } else if (edit.type == EditType::REMOVE) {
            std::cout << "\033[31m" << truncate(edit.text) << "\033[0m < " << std::string(colWidth, ' ') << "\n";
        }
    }
    std::cout << std::string(terminalWidth, '-') << "\n";
}

} // namespace nova::diff
