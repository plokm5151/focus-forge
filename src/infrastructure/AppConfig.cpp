/**
 * @file AppConfig.cpp
 * @brief Implementation of the AppConfig Singleton with JSON persistence.
 *
 * Uses QJsonDocument for serialization and QStandardPaths for platform-
 * appropriate storage locations. Reads on construction, writes on mutation.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "AppConfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

#include <stdexcept>

namespace brain::infrastructure {

// ---------------------------------------------------------------------------
// JSON Key Constants
// ---------------------------------------------------------------------------
namespace {

constexpr auto kConfigFileName      = "config.json";
constexpr auto kKeyFocusDuration    = "focusDurationMinutes";
constexpr auto kKeyCoolDownDuration = "coolDownDurationMinutes";
constexpr auto kKeyTotalPoints      = "totalPoints";
constexpr auto kKeyVaultPath        = "obsidianVaultPath";

/**
 * @brief Resolves the full path to config.json.
 *
 * Uses QStandardPaths::AppDataLocation to place the file in a
 * platform-appropriate directory:
 * - macOS:   ~/Library/Application Support/<AppName>/config.json
 * - Linux:   ~/.local/share/<AppName>/config.json
 * - Windows: C:/Users/<User>/AppData/Local/<AppName>/config.json
 */
[[nodiscard]] auto resolveConfigPath() -> QString {
    const QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(dataDir).filePath(QLatin1String(kConfigFileName));
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Singleton Access
// ---------------------------------------------------------------------------

auto AppConfig::instance() -> AppConfig& {
    // C++11 §6.7/4: thread-safe lazy initialization guaranteed by the standard.
    static AppConfig s_instance;
    return s_instance;
}

// ---------------------------------------------------------------------------
// Constructor — Load from JSON or Use Defaults
// ---------------------------------------------------------------------------

AppConfig::AppConfig()
    : m_focusDurationMinutes{40}    // 40 minutes focus
    , m_coolDownDurationMinutes{10} // 10 minutes cooldown
    , m_totalPoints{0}
    , m_obsidianVaultPath{}
{
    load();
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

auto AppConfig::focusDurationMinutes() const noexcept -> std::int32_t {
    return m_focusDurationMinutes;
}

auto AppConfig::coolDownDurationMinutes() const noexcept -> std::int32_t {
    return m_coolDownDurationMinutes;
}

auto AppConfig::obsidianVaultPath() const noexcept -> std::string_view {
    return m_obsidianVaultPath;
}

auto AppConfig::totalPoints() const noexcept -> std::int32_t {
    return m_totalPoints;
}

// ---------------------------------------------------------------------------
// Mutators (each triggers a save to disk)
// ---------------------------------------------------------------------------

void AppConfig::setFocusDurationMinutes(std::int32_t minutes) {
    if (minutes <= 0) {
        throw std::invalid_argument(
            "AppConfig: focusDurationMinutes must be positive");
    }
    m_focusDurationMinutes = minutes;
    save();
}

void AppConfig::setCoolDownDurationMinutes(std::int32_t minutes) {
    if (minutes <= 0) {
        throw std::invalid_argument(
            "AppConfig: coolDownDurationMinutes must be positive");
    }
    m_coolDownDurationMinutes = minutes;
    save();
}

void AppConfig::setObsidianVaultPath(std::string_view path) {
    m_obsidianVaultPath = std::string{path};
    save();
}

void AppConfig::setTotalPoints(std::int32_t points) {
    if (points < 0) return;
    m_totalPoints = points;
    save();
}

// ---------------------------------------------------------------------------
// JSON Persistence
// ---------------------------------------------------------------------------

void AppConfig::load() {
    const QString path = resolveConfigPath();
    QFile file{path};

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // File not found or unreadable → retain defaults
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return; // Malformed JSON → retain defaults
    }

    const QJsonObject obj = doc.object();

    if (obj.contains(QLatin1String(kKeyFocusDuration))) {
        const int val = obj[QLatin1String(kKeyFocusDuration)].toInt(-1);
        if (val > 0) {
            m_focusDurationMinutes = static_cast<std::int32_t>(val);
        }
    }

    if (obj.contains(QLatin1String(kKeyCoolDownDuration))) {
        const int val = obj[QLatin1String(kKeyCoolDownDuration)].toInt(-1);
        if (val > 0) {
            m_coolDownDurationMinutes = static_cast<std::int32_t>(val);
        }
    }

    if (obj.contains(QLatin1String(kKeyVaultPath))) {
        const QString val = obj[QLatin1String(kKeyVaultPath)].toString();
        if (!val.isEmpty()) {
            m_obsidianVaultPath = val.toStdString();
        }
    }

    if (obj.contains(QLatin1String(kKeyTotalPoints))) {
        const int val = obj[QLatin1String(kKeyTotalPoints)].toInt(-1);
        if (val >= 0) {
            m_totalPoints = static_cast<std::int32_t>(val);
        }
    }
}

void AppConfig::save() const {
    const QString path = resolveConfigPath();

    // Ensure the parent directory exists
    const QFileInfo fileInfo{path};
    QDir().mkpath(fileInfo.absolutePath());

    QJsonObject obj;
    obj[QLatin1String(kKeyFocusDuration)]    = m_focusDurationMinutes;
    obj[QLatin1String(kKeyCoolDownDuration)] = m_coolDownDurationMinutes;
    obj[QLatin1String(kKeyTotalPoints)]      = m_totalPoints;
    obj[QLatin1String(kKeyVaultPath)]        =
        QString::fromStdString(m_obsidianVaultPath);

    QFile file{path};
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return; // Write failure — silently degrade (logging in future phase)
    }

    file.write(QJsonDocument{obj}.toJson(QJsonDocument::Indented));
    file.close();
}

} // namespace brain::infrastructure
