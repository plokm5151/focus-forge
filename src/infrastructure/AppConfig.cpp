/**
 * @file AppConfig.cpp
 * @brief Implementation of the AppConfig thread-safe Singleton.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "AppConfig.h"

#include <stdexcept>

namespace brain::infrastructure {

// ---------------------------------------------------------------------------
// Singleton Access
// ---------------------------------------------------------------------------

auto AppConfig::instance() -> AppConfig& {
    // C++11 §6.7/4: If control enters the declaration concurrently while
    // the variable is being initialized, the concurrent execution shall
    // wait for completion of the initialization.
    static AppConfig s_instance;
    return s_instance;
}

// ---------------------------------------------------------------------------
// Constructor — Default Pomodoro Settings
// ---------------------------------------------------------------------------

AppConfig::AppConfig()
    : m_focusDurationSeconds{1500}   // 25 minutes
    , m_shortBreakSeconds{300}       // 5 minutes
    , m_longBreakSeconds{900}        // 15 minutes
    , m_obsidianVaultPath{}
{
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

auto AppConfig::focusDurationSeconds() const noexcept -> std::int32_t {
    return m_focusDurationSeconds;
}

auto AppConfig::shortBreakSeconds() const noexcept -> std::int32_t {
    return m_shortBreakSeconds;
}

auto AppConfig::longBreakSeconds() const noexcept -> std::int32_t {
    return m_longBreakSeconds;
}

auto AppConfig::obsidianVaultPath() const noexcept -> std::string_view {
    return m_obsidianVaultPath;
}

// ---------------------------------------------------------------------------
// Mutators
// ---------------------------------------------------------------------------

void AppConfig::setFocusDurationSeconds(std::int32_t seconds) {
    if (seconds <= 0) {
        throw std::invalid_argument(
            "AppConfig: focusDurationSeconds must be positive");
    }
    m_focusDurationSeconds = seconds;
}

void AppConfig::setShortBreakSeconds(std::int32_t seconds) {
    if (seconds <= 0) {
        throw std::invalid_argument(
            "AppConfig: shortBreakSeconds must be positive");
    }
    m_shortBreakSeconds = seconds;
}

void AppConfig::setLongBreakSeconds(std::int32_t seconds) {
    if (seconds <= 0) {
        throw std::invalid_argument(
            "AppConfig: longBreakSeconds must be positive");
    }
    m_longBreakSeconds = seconds;
}

void AppConfig::setObsidianVaultPath(std::string_view path) {
    m_obsidianVaultPath = std::string{path};
}

} // namespace brain::infrastructure
