/**
 * @file ObsidianSync.h
 * @brief Concrete implementation of INoteSync for Obsidian vault integration.
 *
 * Provides file-based synchronization to an Obsidian vault directory,
 * appending focus session logs to a designated markdown file.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#ifndef BRAIN_MAINTENANCE_INFRASTRUCTURE_OBSIDIANSYNC_H
#define BRAIN_MAINTENANCE_INFRASTRUCTURE_OBSIDIANSYNC_H

#include "domain/INoteSync.h"

#include <string>
#include <string_view>

namespace brain::infrastructure {

/**
 * @class ObsidianSync
 * @brief Synchronizes focus session data to an Obsidian vault.
 *
 * Implements the INoteSync strategy by appending timestamped text entries
 * to a markdown log file within the configured Obsidian vault directory.
 *
 * @par Design Rationale
 * - Concrete strategy in the infrastructure layer, depending only on the
 *   domain-layer abstraction (INoteSync).
 * - Uses filesystem I/O for local vault access; no network dependency.
 *
 * @par Thread Safety
 * - `syncText()` is NOT thread-safe. Callers must serialize concurrent writes.
 *
 * @see brain::domain::INoteSync
 */
class ObsidianSync final : public brain::domain::INoteSync {
public:
    /**
     * @brief Constructs an ObsidianSync targeting the specified vault path.
     *
     * @param vaultPath Absolute path to the Obsidian vault directory.
     *                  The directory must exist and be writable.
     */
    explicit ObsidianSync(std::string_view vaultPath);

    /**
     * @brief Default destructor.
     */
    ~ObsidianSync() override = default;

    /**
     * @brief Appends text to the focus log file in the Obsidian vault.
     *
     * Creates the log file if it does not exist. Appends a timestamped
     * entry with the provided text content.
     *
     * @param text The text content to append to the log.
     * @return true  if the write operation succeeded.
     * @return false if the vault path is invalid or the write failed.
     */
    [[nodiscard]] auto syncText(std::string_view text) -> bool override;

    /**
     * @brief Appends a task to FocusTasks.md in the vault.
     * @param taskText The text of the task to append.
     */
    void appendTodo(std::string_view taskText) override;
    
    [[nodiscard]] auto readTasks() const -> std::vector<TaskItem> override;
    void updateTask(int index, bool isCompleted) override;

private:
    std::string m_vaultPath;     ///< Absolute path to the Obsidian vault.
    std::string m_logFileName;   ///< Name of the focus session log file.
};

} // namespace brain::infrastructure

#endif // BRAIN_MAINTENANCE_INFRASTRUCTURE_OBSIDIANSYNC_H
