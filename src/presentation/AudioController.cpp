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

namespace brain::presentation {

using brain::domain::TimerState;

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
    QString appDir = QCoreApplication::applicationDirPath();
    m_bellSound->setSource(QUrl::fromLocalFile(appDir + QStringLiteral("/assets/audio/bell.wav")));
    
    // Default volume (can be configured via AppConfig in the future)
    m_bgOutput->setVolume(0.5f);
    m_bellSound->setVolume(1.0f);
}

void AudioController::onTimerStateChanged(TimerState newState, TimerState oldState) {
    Q_UNUSED(oldState);

    QString appDir = QCoreApplication::applicationDirPath();

    // The bell always chimes on any state transition triggered by the system or user
    m_bellSound->play();

    if (newState == TimerState::Focusing) {
        m_bgPlayer->setSource(QUrl::fromLocalFile(appDir + QStringLiteral("/assets/audio/focus.ogg")));
        m_bgPlayer->play();
    } else if (newState == TimerState::CoolDown) {
        m_bgPlayer->setSource(QUrl::fromLocalFile(appDir + QStringLiteral("/assets/audio/cooldown.mp3")));
        m_bgPlayer->play();
    } else if (newState == TimerState::Idle) {
        m_bgPlayer->stop();
    }
}

} // namespace brain::presentation
