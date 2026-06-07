/**
 * @file INoteSync.h
 * @brief Defines the abstract interface for note synchronization strategies.
 *
 * This header establishes the contract for the Strategy pattern, enabling
 * Dependency Inversion between the domain layer and concrete infrastructure
 * implementations. All synchronization backends must implement this interface.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#ifndef BRAIN_MAINTENANCE_DOMAIN_INOTESYNC_H
#define BRAIN_MAINTENANCE_DOMAIN_INOTESYNC_H

#include <string_view>
#include <string>
#include <vector>

namespace brain::domain {

/**
 * @interface INoteSync
 * @brief Pure virtual interface for note synchronization.
 *
 * Provides a strategy contract that decouples the application's core logic
 * from any specific note-taking or synchronization backend. Concrete
 * implementations (e.g., ObsidianSync, NotionSync) reside in the
 * infrastructure layer.
 *
 * @par Design Rationale
 * - Follows the Dependency Inversion Principle (DIP): high-level modules
 *   depend on this abstraction, not on concrete implementations.
 * - Follows the Interface Segregation Principle (ISP): the interface
 *   exposes only the minimal synchronization contract.
 *
 * @par Thread Safety
 * Implementations must document their own thread-safety guarantees.
 *
 * @see brain::infrastructure::ObsidianSync
 */
class INoteSync {
public:
    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~INoteSync() = default;

    /**
     * @brief Synchronizes the given text content to the target backend.
     *
     * @param text A non-owning view of the text content to synchronize.
     *             The caller must ensure the underlying data outlives this call.
     * @return true  if the synchronization completed successfully.
     * @return false if the synchronization failed (e.g., network error,
     *               file write failure).
     *
     * @throws std::runtime_error May throw on unrecoverable I/O errors,
     *         depending on the implementation.
     *
     * @par Example
     * @code
     * auto sync = std::make_shared<ObsidianSync>();
     * bool ok = sync->syncText("Focus session completed: 25 min");
     * @endcode
     */
    [[nodiscard]] virtual auto syncText(std::string_view text) -> bool = 0;

    struct TaskItem {
        std::string text;
        bool isCompleted;
        std::string dueDate;
        int priority; // 3 = High, 2 = Normal, 1 = Low
    };

    /**
     * @brief Appends a quick capture task to a predefined Obsidian note.
     * @param task The task object to append.
     */
    virtual void appendTodo(const TaskItem& task) = 0;

    /**
     * @brief Reads tasks from the sync target.
     *
     * @return A vector of parsed tasks.
     */
    [[nodiscard]] virtual auto readTasks() const -> std::vector<TaskItem> = 0;

    /**
     * @brief Updates the completion state of a specific task by index.
     *
     * @param index The zero-based index of the task.
     * @param isCompleted The new completion state.
     */
    virtual void updateTask(int index, bool isCompleted) = 0;

    /**
     * @brief Updates the text and metadata of an existing task.
     * @param index The zero-based index of the task.
     * @param task The task containing the new text and metadata.
     */
    virtual void updateTaskText(int index, const TaskItem& task) = 0;

    /**
     * @brief Deletes a task.
     * @param index The zero-based index of the task.
     */
    virtual void deleteTask(int index) = 0;

    /**
     * @brief Clears all tasks.
     */
    virtual void clearAllTasks() = 0;

    // --- Deleted copy/move to prevent slicing ---
    INoteSync(const INoteSync&) = delete;
    INoteSync& operator=(const INoteSync&) = delete;
    INoteSync(INoteSync&&) = delete;
    INoteSync& operator=(INoteSync&&) = delete;

protected:
    /**
     * @brief Protected default constructor; only derived classes instantiate.
     */
    INoteSync() = default;
};

} // namespace brain::domain

#endif // BRAIN_MAINTENANCE_DOMAIN_INOTESYNC_H
