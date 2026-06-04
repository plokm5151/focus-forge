/**
 * @file AudioController.cpp
 * @brief Implementation of the AudioController for managing Qt6 Multimedia playback.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "AudioController.h"

#include <QCoreApplication>
#include <QUrl>
#include <QString>

namespace brain::presentation {

using brain::domain::TimerState;

// Helper to correctly resolve asset paths regardless of whether running locally or inside a macOS .app bundle.
static auto resolveAssetPath(const QString& relativePath) -> QString {
    QString appDir = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
    if (appDir.endsWith(QStringLiteral("Contents/MacOS"))) {
        // App is bundled, assets are in Contents/Resources
        QString resourceDir = appDir;
        resourceDir.replace(QStringLiteral("Contents/MacOS"), QStringLiteral("Contents/Resources"));
        return resourceDir + QStringLiteral("/") + relativePath;
    }
#endif
    return appDir + QStringLiteral("/") + relativePath;
}

AudioController::AudioController(QObject* parent)
    : QObject{parent}
    , m_bgPlayer{new QMediaPlayer{this}}
    , m_bgOutput{new QAudioOutput{this}}
    , m_bellSound{new QSoundEffect{this}}
{
    // Setup background looping audio
    m_bgPlayer->setAudioOutput(m_bgOutput);
    m_bgPlayer->setLoops(QMediaPlayer::Infinite);
    
    // Pre-load the WAV file for zero-latency bell
    m_bellSound->setSource(QUrl::fromLocalFile(resolveAssetPath(QStringLiteral("assets/audio/bell.wav"))));
    
    // Default volume (can be configured via AppConfig in the future)
    m_bgOutput->setVolume(0.5f);
    m_bellSound->setVolume(1.0f);
}

void AudioController::onTimerStateChanged(TimerState newState, TimerState oldState) {
    Q_UNUSED(oldState);

    // The bell always chimes on any state transition triggered by the system or user
    m_bellSound->play();

    if (newState == TimerState::Focusing) {
        m_bgPlayer->setSource(QUrl::fromLocalFile(resolveAssetPath(QStringLiteral("assets/audio/focus.ogg"))));
        m_bgPlayer->play();
    } else if (newState == TimerState::CoolDown) {
        m_bgPlayer->setSource(QUrl::fromLocalFile(resolveAssetPath(QStringLiteral("assets/audio/cooldown.mp3"))));
        m_bgPlayer->play();
    } else if (newState == TimerState::Idle) {
        m_bgPlayer->stop();
    }
}

} // namespace brain::presentation
