/**
 * @file ObsidianSync.cpp
 * @brief Implementation of Obsidian vault synchronization strategy.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "ObsidianSync.h"

#include <QDateTime>
#include <QTimeZone>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <QtConcurrent>
#include <QFuture>

namespace brain::infrastructure {

namespace fs = std::filesystem;

namespace {
    static std::mutex s_obsidianMutex;
}

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

    std::string textCopy{text};
    std::string vault = m_vaultPath;
    std::string logName = m_logFileName;

    (void)QtConcurrent::run([textCopy, vault, logName]() {
        std::lock_guard<std::mutex> lock(s_obsidianMutex);
        
        const fs::path vaultDir{vault};
        if (!fs::exists(vaultDir) || !fs::is_directory(vaultDir)) {
            std::error_code ec;
            fs::create_directories(vaultDir, ec);
            if (ec) return;
        }

        const fs::path logFilePath = vaultDir / logName;

        std::ofstream ofs{logFilePath, std::ios::app};
        if (!ofs.is_open()) return;

        ofs << "- " << textCopy << '\n';
        ofs.flush();
    });

    return true;
}

static std::string formatTaskLine(const brain::domain::INoteSync::TaskItem& task) {
    std::string_view priorityStr;
    if (task.priority == 3) priorityStr = " 🔺";
    else if (task.priority == 2) priorityStr = " 🔼";
    else if (task.priority == 1) priorityStr = " 🔽";
    
    std::string dateStr;
    if (!task.dueDate.empty()) {
        dateStr = std::format(" 📅 {}", task.dueDate);
    }
    
    return std::format("- [{}] {}{}{}", 
                       task.isCompleted ? 'x' : ' ', 
                       task.text, 
                       priorityStr, 
                       dateStr);
}

void ObsidianSync::appendTodo(const TaskItem& task) {
    if (m_vaultPath.empty()) {
        std::cerr << "[ObsidianSync] Error: vault path is empty.\n";
        return;
    }

    std::string vault = m_vaultPath;
    TaskItem taskCopy = task;

    (void)QtConcurrent::run([vault, taskCopy]() {
        std::lock_guard<std::mutex> lock(s_obsidianMutex);
        
        const fs::path vaultDir{vault};
        if (!fs::exists(vaultDir) || !fs::is_directory(vaultDir)) {
            std::error_code ec;
            fs::create_directories(vaultDir, ec);
            if (ec) return;
        }

        const fs::path taskFilePath = vaultDir / "FocusTasks.md";

        std::ofstream ofs{taskFilePath, std::ios::app};
        if (!ofs.is_open()) return;

        ofs << formatTaskLine(taskCopy) << '\n';
        ofs.flush();
    });
}

auto ObsidianSync::readTasks() const -> std::vector<TaskItem> {
    std::vector<TaskItem> tasks;
    if (m_vaultPath.empty()) return tasks;

    tasks.reserve(50); // Pre-allocate to reduce vector reallocations

    std::lock_guard<std::mutex> lock(s_obsidianMutex);

    const fs::path taskFilePath = fs::path{m_vaultPath} / "FocusTasks.md";
    std::ifstream ifs{taskFilePath};
    if (!ifs.is_open()) return tasks;

    std::string line;
    while (std::getline(ifs, line)) {
        bool isCompleted = false;
        std::string rawText;
        if (line.starts_with("- [ ] ")) {
            rawText = line.substr(6);
        } else if (line.starts_with("- [x] ") || line.starts_with("- [X] ")) {
            rawText = line.substr(6);
            isCompleted = true;
        } else {
            continue;
        }

        TaskItem item;
        item.isCompleted = isCompleted;
        item.priority = 0; // Default

        // Parse dueDate: 📅 YYYY-MM-DD
        if (auto pos = rawText.find("📅 "); pos != std::string::npos) {
            if (pos + 14 <= rawText.length()) { // "📅 " + 10 chars
                item.dueDate = rawText.substr(pos + 5, 10);
            }
            rawText.erase(pos); // remove everything after and including 📅
        }

        // Parse priority
        if (auto pos = rawText.find("🔺"); pos != std::string::npos) {
            item.priority = 3;
            rawText.erase(pos);
        } else if (auto pos2 = rawText.find("🔼"); pos2 != std::string::npos) {
            item.priority = 2;
            rawText.erase(pos2);
        } else if (auto pos3 = rawText.find("🔽"); pos3 != std::string::npos) {
            item.priority = 1;
            rawText.erase(pos3);
        }

        // Trim trailing whitespaces
        while (!rawText.empty() && std::isspace(rawText.back())) {
            rawText.pop_back();
        }
        item.text = rawText;
        tasks.push_back(item);
    }
    return tasks;
}

void ObsidianSync::updateTask(int index, bool isCompleted) {
    if (m_vaultPath.empty() || index < 0) return;

    std::string vault = m_vaultPath;

    (void)QtConcurrent::run([vault, index, isCompleted]() {
        std::lock_guard<std::mutex> lock(s_obsidianMutex);
        
        const fs::path taskFilePath = fs::path{vault} / "FocusTasks.md";
        const fs::path tmpFilePath = fs::path{vault} / "FocusTasks.tmp.md";
        
        std::ifstream ifs{taskFilePath};
        if (!ifs.is_open()) return;

        std::ofstream ofs{tmpFilePath, std::ios::trunc};
        if (!ofs.is_open()) return;

        std::string line;
        int currentTaskIndex = 0;

        while (std::getline(ifs, line)) {
            if (line.starts_with("- [ ] ") || line.starts_with("- [x] ") || line.starts_with("- [X] ")) {
                if (currentTaskIndex == index) {
                    std::string_view prefix = isCompleted ? "- [x] " : "- [ ] ";
                    line = std::format("{}{}", prefix, line.substr(6));
                }
                currentTaskIndex++;
            }
            ofs << line << '\n';
        }
        
        ifs.close();
        ofs.flush();
        ofs.close();

        std::error_code ec;
        fs::rename(tmpFilePath, taskFilePath, ec);
    });
}

void ObsidianSync::updateTaskText(int index, const TaskItem& task) {
    if (m_vaultPath.empty() || index < 0) return;

    std::string vault = m_vaultPath;
    TaskItem taskCopy = task;

    (void)QtConcurrent::run([vault, index, taskCopy]() {
        std::lock_guard<std::mutex> lock(s_obsidianMutex);
        
        const fs::path taskFilePath = fs::path{vault} / "FocusTasks.md";
        const fs::path tmpFilePath = fs::path{vault} / "FocusTasks.tmp.md";
        
        std::ifstream ifs{taskFilePath};
        if (!ifs.is_open()) return;

        std::ofstream ofs{tmpFilePath, std::ios::trunc};
        if (!ofs.is_open()) return;

        std::string line;
        int currentTaskIndex = 0;

        while (std::getline(ifs, line)) {
            if (line.starts_with("- [ ] ") || line.starts_with("- [x] ") || line.starts_with("- [X] ")) {
                if (currentTaskIndex == index) {
                    line = formatTaskLine(taskCopy);
                }
                currentTaskIndex++;
            }
            ofs << line << '\n';
        }
        
        ifs.close();
        ofs.flush();
        ofs.close();

        std::error_code ec;
        fs::rename(tmpFilePath, taskFilePath, ec);
    });
}

void ObsidianSync::deleteTask(int index) {
    if (m_vaultPath.empty() || index < 0) return;

    std::string vault = m_vaultPath;

    (void)QtConcurrent::run([vault, index]() {
        std::lock_guard<std::mutex> lock(s_obsidianMutex);
        
        const fs::path taskFilePath = fs::path{vault} / "FocusTasks.md";
        const fs::path tmpFilePath = fs::path{vault} / "FocusTasks.tmp.md";
        
        std::ifstream ifs{taskFilePath};
        if (!ifs.is_open()) return;

        std::ofstream ofs{tmpFilePath, std::ios::trunc};
        if (!ofs.is_open()) return;

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
            ofs << line << '\n';
        }
        
        ifs.close();
        ofs.flush();
        ofs.close();

        std::error_code ec;
        fs::rename(tmpFilePath, taskFilePath, ec);
    });
}

void ObsidianSync::clearAllTasks() {
    std::string vault = m_vaultPath;

    (void)QtConcurrent::run([vault]() {
        std::lock_guard<std::mutex> lock(s_obsidianMutex);
        
        const fs::path taskFilePath = fs::path{vault} / "FocusTasks.md";
        const fs::path tmpFilePath = fs::path{vault} / "FocusTasks.tmp.md";
        
        try {
            // Write an empty file atomically
            std::ofstream outFile(tmpFilePath, std::ios::trunc);
            if (!outFile) {
                std::cerr << "[ObsidianSync] Error: Could not truncate FocusTasks.md\n";
                return;
            }
            outFile.flush();
            outFile.close();

            std::error_code ec;
            fs::rename(tmpFilePath, taskFilePath, ec);
        } catch (const std::exception& e) {
            std::cerr << "[ObsidianSync] Exception truncating tasks: " << e.what() << "\n";
        }
    });
}

} // namespace brain::infrastructure
