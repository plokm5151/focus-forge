/**
 * @file AppConfig.h
 * @brief Thread-safe Singleton for application-wide configuration management.
 *
 * Provides centralized, lazy-initialized access to application settings
 * using C++11 magic statics (Meyers' Singleton) for guaranteed thread safety.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#ifndef BRAIN_MAINTENANCE_INFRASTRUCTURE_APPCONFIG_H
#define BRAIN_MAINTENANCE_INFRASTRUCTURE_APPCONFIG_H

#include <cstdint>
#include <string>
#include <string_view>

namespace brain::infrastructure {

/**
 * @class AppConfig
 * @brief Singleton configuration manager for the application.
 *
 * Manages application-wide settings such as focus duration, break intervals,
 * and synchronization paths. Uses C++11 magic statics (§6.7/4) to guarantee
 * thread-safe, lazy initialization without explicit locking.
 *
 * @par Design Rationale
 * - Meyers' Singleton: `static` local in `instance()` ensures exactly one
 *   construction, thread-safe per the C++ standard.
 * - Non-copyable, non-movable to prevent accidental duplication.
 *
 * @par Thread Safety
 * - `instance()`: Thread-safe (guaranteed by the standard).
 * - Accessor methods: Read-only accessors are safe for concurrent reads.
 *   Mutators require external synchronization if called from multiple threads.
 *
 * @par Usage
 * @code
 * auto& config = AppConfig::instance();
 * int duration = config.focusDurationSeconds();
 * config.setObsidianVaultPath("/path/to/vault");
 * @endcode
 */
class AppConfig {
public:
    /**
     * @brief Returns the singleton instance.
     *
     * Uses C++11 magic statics for thread-safe lazy initialization.
     * The instance is created on first call and destroyed at program exit.
     *
     * @return Reference to the single AppConfig instance.
     */
    [[nodiscard]] static auto instance() -> AppConfig&;

    // --- Deleted copy/move semantics ---
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    AppConfig(AppConfig&&) = delete;
    AppConfig& operator=(AppConfig&&) = delete;

    // --- Accessors ---

    /**
     * @brief Gets the default focus session duration.
     * @return Duration in seconds (default: 1500 = 25 minutes).
     */
    [[nodiscard]] auto focusDurationSeconds() const noexcept -> std::int32_t;

    /**
     * @brief Gets the short break duration.
     * @return Duration in seconds (default: 300 = 5 minutes).
     */
    [[nodiscard]] auto shortBreakSeconds() const noexcept -> std::int32_t;

    /**
     * @brief Gets the long break duration.
     * @return Duration in seconds (default: 900 = 15 minutes).
     */
    [[nodiscard]] auto longBreakSeconds() const noexcept -> std::int32_t;

    /**
     * @brief Gets the configured Obsidian vault path for sync.
     * @return The vault directory path as a string view.
     */
    [[nodiscard]] auto obsidianVaultPath() const noexcept -> std::string_view;

    // --- Mutators ---

    /**
     * @brief Sets the focus session duration.
     * @param seconds Duration in seconds. Must be > 0.
     */
    void setFocusDurationSeconds(std::int32_t seconds);

    /**
     * @brief Sets the short break duration.
     * @param seconds Duration in seconds. Must be > 0.
     */
    void setShortBreakSeconds(std::int32_t seconds);

    /**
     * @brief Sets the long break duration.
     * @param seconds Duration in seconds. Must be > 0.
     */
    void setLongBreakSeconds(std::int32_t seconds);

    /**
     * @brief Sets the Obsidian vault directory path.
     * @param path Absolute path to the Obsidian vault.
     */
    void setObsidianVaultPath(std::string_view path);

private:
    /**
     * @brief Private constructor; initializes default configuration values.
     */
    AppConfig();

    /**
     * @brief Default destructor.
     */
    ~AppConfig() = default;

    std::int32_t m_focusDurationSeconds;  ///< Focus session length in seconds.
    std::int32_t m_shortBreakSeconds;     ///< Short break length in seconds.
    std::int32_t m_longBreakSeconds;      ///< Long break length in seconds.
    std::string  m_obsidianVaultPath;     ///< Path to Obsidian vault directory.
};

} // namespace brain::infrastructure

#endif // BRAIN_MAINTENANCE_INFRASTRUCTURE_APPCONFIG_H
