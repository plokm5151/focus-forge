/**
 * @file ObsidianSync.cpp
 * @brief Implementation of Obsidian vault synchronization strategy.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "ObsidianSync.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

namespace brain::infrastructure {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ObsidianSync::ObsidianSync(std::string_view vaultPath)
    : m_vaultPath{vaultPath}
    , m_logFileName{"brain-maintenance-log.md"}
{
}

// ---------------------------------------------------------------------------
// INoteSync Implementation
// ---------------------------------------------------------------------------

auto ObsidianSync::syncText(std::string_view text) -> bool {
    if (m_vaultPath.empty()) {
        std::cerr << "[ObsidianSync] Error: vault path is empty.\n";
        return false;
    }

    const fs::path vaultDir{m_vaultPath};
    if (!fs::exists(vaultDir) || !fs::is_directory(vaultDir)) {
        std::cerr << "[ObsidianSync] Error: vault path does not exist or "
                     "is not a directory: "
                  << m_vaultPath << '\n';
        return false;
    }

    const fs::path logFilePath = vaultDir / m_logFileName;

    // Open in append mode; creates the file if it does not exist.
    std::ofstream ofs{logFilePath, std::ios::app};
    if (!ofs.is_open()) {
        std::cerr << "[ObsidianSync] Error: failed to open log file: "
                  << logFilePath << '\n';
        return false;
    }

    // Generate ISO 8601 timestamp
    const auto now = std::chrono::system_clock::now();
    const auto timeT = std::chrono::system_clock::to_time_t(now);
    const std::tm* localTime = std::localtime(&timeT);

    char timestamp[64]{};
    std::strftime(timestamp, sizeof(timestamp),
                  "%Y-%m-%dT%H:%M:%S", localTime);

    // Write timestamped markdown entry
    ofs << "- **[" << timestamp << "]** " << text << '\n';
    ofs.flush();

    if (ofs.fail()) {
        std::cerr << "[ObsidianSync] Error: write to log file failed.\n";
        return false;
    }

    return true;
}

void ObsidianSync::appendTodo(std::string_view taskText) {
    if (m_vaultPath.empty()) {
        std::cerr << "[ObsidianSync] Error: vault path is empty.\n";
        return;
    }

    const fs::path vaultDir{m_vaultPath};
    if (!fs::exists(vaultDir) || !fs::is_directory(vaultDir)) {
        std::cerr << "[ObsidianSync] Error: vault path does not exist or is not a directory: " << m_vaultPath << '\n';
        return;
    }

    const fs::path taskFilePath = vaultDir / "FocusTasks.md";

    std::ofstream ofs{taskFilePath, std::ios::app};
    if (!ofs.is_open()) {
        std::cerr << "[ObsidianSync] Error: failed to open task file: " << taskFilePath << '\n';
        return;
    }

    ofs << "- [ ] " << taskText << '\n';
    ofs.flush();
}

auto ObsidianSync::readTasks() const -> std::vector<TaskItem> {
    std::vector<TaskItem> tasks;
    if (m_vaultPath.empty()) return tasks;

    const fs::path taskFilePath = fs::path{m_vaultPath} / "FocusTasks.md";
    std::ifstream ifs{taskFilePath};
    if (!ifs.is_open()) return tasks;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.starts_with("- [ ] ")) {
            tasks.push_back(TaskItem{line.substr(6), false});
        } else if (line.starts_with("- [x] ")) {
            tasks.push_back(TaskItem{line.substr(6), true});
        } else if (line.starts_with("- [X] ")) {
            tasks.push_back(TaskItem{line.substr(6), true});
        }
    }
    return tasks;
}

void ObsidianSync::updateTask(int index, bool isCompleted) {
    if (m_vaultPath.empty() || index < 0) return;

    const fs::path taskFilePath = fs::path{m_vaultPath} / "FocusTasks.md";
    std::ifstream ifs{taskFilePath};
    if (!ifs.is_open()) return;

    std::vector<std::string> lines;
    std::string line;
    int currentTaskIndex = 0;

    while (std::getline(ifs, line)) {
        if (line.starts_with("- [ ] ") || line.starts_with("- [x] ") || line.starts_with("- [X] ")) {
            if (currentTaskIndex == index) {
                // Update this task line
                std::string prefix = isCompleted ? "- [x] " : "- [ ] ";
                line = prefix + line.substr(6);
            }
            currentTaskIndex++;
        }
        lines.push_back(line);
    }
    ifs.close();

    std::ofstream ofs{taskFilePath, std::ios::trunc};
    if (!ofs.is_open()) return;

    for (const auto& l : lines) {
        ofs << l << '\n';
    }
}

void ObsidianSync::updateTaskText(int index, std::string_view newText) {
    if (m_vaultPath.empty() || index < 0) return;

    const fs::path taskFilePath = fs::path{m_vaultPath} / "FocusTasks.md";
    std::ifstream ifs{taskFilePath};
    if (!ifs.is_open()) return;

    std::vector<std::string> lines;
    std::string line;
    int currentTaskIndex = 0;

    while (std::getline(ifs, line)) {
        if (line.starts_with("- [ ] ") || line.starts_with("- [x] ") || line.starts_with("- [X] ")) {
            if (currentTaskIndex == index) {
                line = line.substr(0, 6) + std::string(newText);
            }
            currentTaskIndex++;
        }
        lines.push_back(line);
    }
    ifs.close();

    std::ofstream ofs{taskFilePath, std::ios::trunc};
    if (!ofs.is_open()) return;

    for (const auto& l : lines) {
        ofs << l << '\n';
    }
}

void ObsidianSync::deleteTask(int index) {
    if (m_vaultPath.empty() || index < 0) return;

    const fs::path taskFilePath = fs::path{m_vaultPath} / "FocusTasks.md";
    std::ifstream ifs{taskFilePath};
    if (!ifs.is_open()) return;

    std::vector<std::string> lines;
    std::string line;
    int currentTaskIndex = 0;

    while (std::getline(ifs, line)) {
        if (line.starts_with("- [ ] ") || line.starts_with("- [x] ") || line.starts_with("- [X] ")) {
            if (currentTaskIndex == index) {
                currentTaskIndex++;
                continue; // Skip this line to delete it
            }
            currentTaskIndex++;
        }
        lines.push_back(line);
    }
    ifs.close();

    std::ofstream ofs{taskFilePath, std::ios::trunc};
    if (!ofs.is_open()) return;

    for (const auto& l : lines) {
        ofs << l << '\n';
    }
}

} // namespace brain::infrastructure
