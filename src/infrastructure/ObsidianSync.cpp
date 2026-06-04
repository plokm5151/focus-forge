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

} // namespace brain::infrastructure
