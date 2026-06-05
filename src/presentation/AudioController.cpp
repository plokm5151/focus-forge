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
#include <QDebug>

namespace brain::presentation {

using brain::domain::TimerState;

AudioController::AudioController(QObject* parent)
    : QObject{parent}
    , m_bgPlayer{new QMediaPlayer{this}}
    , m_bgOutput{new QAudioOutput{this}}
    , m_bellPlayer{new QMediaPlayer{this}}
    , m_bellOutput{new QAudioOutput{this}}
    , m_clickSound{new QSoundEffect{this}}
    , m_fadeTimer{new QTimer{this}}
{
    // Setup background looping audio
    m_bgPlayer->setAudioOutput(m_bgOutput);
    m_bgPlayer->setLoops(QMediaPlayer::Infinite);
    
    // Setup bell audio
    m_bellPlayer->setAudioOutput(m_bellOutput);

#ifdef Q_OS_MAC
    QString audioDir = QCoreApplication::applicationDirPath() + "/../Resources/assets/audio/";
#else
    QString audioDir = QCoreApplication::applicationDirPath() + "/assets/audio/";
#endif

    m_bellPlayer->setSource(QUrl::fromLocalFile(audioDir + "bell.wav"));
    m_clickSound->setSource(QUrl::fromLocalFile(audioDir + "click.wav"));
    
    // Default volume
    m_bgOutput->setVolume(0.5f);
    m_bellOutput->setVolume(1.0f);
    m_clickSound->setVolume(0.8f);

    // Crossfade timer: fires every 50ms for smooth 2-second fade
    m_fadeTimer->setInterval(50);
    connect(m_fadeTimer, &QTimer::timeout, this, [this]() {
        // Fade out current
        m_fadeCurrentVolume -= 0.025f; // 0.5 / 20 steps = 0.025 per step (1 second fade out)
        if (m_fadeCurrentVolume <= 0.0f) {
            m_fadeCurrentVolume = 0.0f;
            m_fadeTimer->stop();

            // Switch source and fade in
            m_bgPlayer->stop();
            m_bgPlayer->setSource(QUrl::fromLocalFile(m_pendingSource));
            m_bgPlayer->play();
            m_bgOutput->setVolume(m_volume);
            m_pendingSource.clear();
        }
        m_bgOutput->setVolume(m_fadeCurrentVolume);
    });

    connect(m_bellPlayer, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "Bell Player Error:" << error << errorString;
    });
    connect(m_bgPlayer, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "BG Player Error:" << error << errorString;
    });
}

auto AudioController::isMuted() const noexcept -> bool {
    return m_isMuted;
}

void AudioController::setMuted(bool muted) {
    if (m_isMuted == muted) return;
    
    m_isMuted = muted;
    m_bgOutput->setMuted(m_isMuted);
    m_bellOutput->setMuted(m_isMuted);
    m_clickSound->setMuted(m_isMuted);
    emit isMutedChanged(m_isMuted);
}

void AudioController::toggleMute() {
    setMuted(!m_isMuted);
}

auto AudioController::volume() const noexcept -> float {
    return m_volume;
}

void AudioController::setVolume(float volume) {
    if (qFuzzyCompare(m_volume, volume)) return;
    
    m_volume = volume;
    m_bgOutput->setVolume(m_volume);
    emit volumeChanged(m_volume);
}

void AudioController::playClickSound() {
    m_clickSound->play();
}

void AudioController::crossfadeTo(const QString& newSource) {
    m_pendingSource = newSource;
    m_fadeCurrentVolume = m_volume;
    m_fadeTimer->start();
}

void AudioController::onTimerStateChanged(TimerState newState, TimerState oldState) {
    qDebug() << "Audio Transition - Old:" << static_cast<int>(oldState) << "New:" << static_cast<int>(newState);
    
    // Play the bell when a focus session completes or when cooldown completes
    bool isSessionComplete = (oldState == TimerState::Focusing && newState == TimerState::Overtime) ||
                             (oldState == TimerState::CoolDown && newState == TimerState::Idle);

    if (isSessionComplete) {
        m_bellPlayer->stop();
        m_bellPlayer->setPosition(0);
        m_bellPlayer->play();
    }

#ifdef Q_OS_MAC
    QString audioDir = QCoreApplication::applicationDirPath() + "/../Resources/assets/audio/";
#else
    QString audioDir = QCoreApplication::applicationDirPath() + "/assets/audio/";
#endif

    if (newState == TimerState::Focusing) {
        if (oldState != TimerState::Paused) {
            m_bgPlayer->setSource(QUrl::fromLocalFile(audioDir + "focus.mp3"));
        }
        m_bgPlayer->play();
    } else if (newState == TimerState::Overtime) {
        // Keep focus music playing during overtime (flow state)
        // No change needed, music continues
    } else if (newState == TimerState::CoolDown) {
        // Crossfade from focus to cooldown music
        if (oldState == TimerState::Focusing || oldState == TimerState::Overtime) {
            crossfadeTo(audioDir + "cooldown.mp3");
        } else {
            m_bgPlayer->setSource(QUrl::fromLocalFile(audioDir + "cooldown.mp3"));
            m_bgPlayer->play();
        }
    } else if (newState == TimerState::Paused) {
        m_bgPlayer->pause();
    } else if (newState == TimerState::Idle) {
        m_bgPlayer->stop();
    }
}

} // namespace brain::presentation
