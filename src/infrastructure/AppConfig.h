/**
 * @file AppConfig.h
 * @brief Thread-safe Singleton with persistent JSON configuration.
 *
 * Provides centralized, lazy-initialized access to application settings
 * using C++11 magic statics (Meyers' Singleton) for guaranteed thread safety.
 * Configuration is persisted to a `config.json` file at the platform-specific
 * application data location (via QStandardPaths::AppDataLocation).
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
 * @brief Singleton configuration manager with JSON persistence.
 *
 * Manages application-wide settings such as focus duration, cooldown
 * duration, and synchronization paths. Settings are loaded from
 * `config.json` on first access and saved to disk when modified.
 *
 * @par Persistence
 * - File format: JSON (`config.json`)
 * - Location: `QStandardPaths::writableLocation(AppDataLocation)`
 * - Created automatically on first write; missing file → defaults used.
 *
 * @par Design Rationale
 * - Meyers' Singleton: `static` local in `instance()` ensures exactly one
 *   construction, thread-safe per the C++ standard.
 * - Non-copyable, non-movable to prevent accidental duplication.
 * - Qt types (QJsonDocument, QFile) used only in the .cpp to keep the
 *   header Qt-free for maximum portability.
 *
 * @par Thread Safety
 * - `instance()`: Thread-safe (guaranteed by the standard).
 * - Accessor methods: Read-only accessors are safe for concurrent reads.
 *   Mutators require external synchronization if called from multiple threads.
 *
 * @par Usage
 * @code
 * auto& config = AppConfig::instance();
 * int focusMin = config.focusDurationMinutes();  // Default: 40
 * config.setCoolDownDurationMinutes(15);          // Persists to config.json
 * @endcode
 */
class AppConfig {
public:
    /**
     * @brief Returns the singleton instance.
     *
     * Uses C++11 magic statics for thread-safe lazy initialization.
     * On first call, loads settings from `config.json` (or uses defaults).
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
     * @brief Gets the focus session duration in minutes.
     * @return Duration in minutes (default: 40).
     */
    [[nodiscard]] auto focusDurationMinutes() const noexcept -> std::int32_t;

    /**
     * @brief Gets the cooldown (break) duration in minutes.
     * @return Duration in minutes (default: 10).
     */
    [[nodiscard]] auto coolDownDurationMinutes() const noexcept -> std::int32_t;

    /**
     * @brief Gets the configured Obsidian vault path for sync.
     * @return The vault directory path as a string view.
     */
    [[nodiscard]] auto obsidianVaultPath() const noexcept -> std::string_view;

    // --- Mutators (persist to config.json) ---

    /**
     * @brief Sets the focus session duration and persists to disk.
     * @param minutes Duration in minutes. Must be > 0.
     * @throws std::invalid_argument if minutes <= 0.
     */
    void setFocusDurationMinutes(std::int32_t minutes);

    /**
     * @brief Sets the cooldown duration and persists to disk.
     * @param minutes Duration in minutes. Must be > 0.
     * @throws std::invalid_argument if minutes <= 0.
     */
    void setCoolDownDurationMinutes(std::int32_t minutes);

    /**
     * @brief Sets the Obsidian vault directory path and persists to disk.
     * @param path Absolute path to the Obsidian vault.
     */
    void setObsidianVaultPath(std::string_view path);

private:
    /**
     * @brief Private constructor; loads settings from config.json or defaults.
     */
    AppConfig();

    /**
     * @brief Default destructor.
     */
    ~AppConfig() = default;

    /**
     * @brief Loads configuration from the JSON file on disk.
     *
     * If the file does not exist or is malformed, defaults are retained.
     */
    void load();

    /**
     * @brief Saves the current configuration to the JSON file on disk.
     *
     * Creates the parent directory and file if they do not exist.
     */
    void save() const;

    std::int32_t m_focusDurationMinutes;    ///< Focus session length in minutes (default: 40).
    std::int32_t m_coolDownDurationMinutes;  ///< Cooldown length in minutes (default: 10).
    std::string  m_obsidianVaultPath;       ///< Path to Obsidian vault directory.
};

} // namespace brain::infrastructure

#endif // BRAIN_MAINTENANCE_INFRASTRUCTURE_APPCONFIG_H
