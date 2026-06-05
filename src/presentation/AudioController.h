/**
 * @file AudioController.h
 * @brief Manages multimedia playback for focus sessions.
 *
 * Implements audio playback for the dashboard using Qt6::Multimedia.
 * It manages two distinct types of audio:
 * - Looping background ambients (QMediaPlayer) for focus and cooldown.
 * - Zero-latency one-shot sound effects (QSoundEffect) for the transition bell.
 * - Click sound effect for task completion.
 *
 * It listens to timer state transitions from the TimerViewModel.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#ifndef BRAIN_MAINTENANCE_PRESENTATION_AUDIOCONTROLLER_H
#define BRAIN_MAINTENANCE_PRESENTATION_AUDIOCONTROLLER_H

#include "domain/TimerState.h"

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QTimer>

namespace brain::presentation {

/**
 * @class AudioController
 * @brief Presentation layer controller for audio assets.
 *
 * Manages playback of loopable background audio and one-shot transition chimes.
 */
class AudioController : public QObject {
    Q_OBJECT

    /**
     * @property isMuted
     * @brief Indicates whether all audio output is muted.
     */
    Q_PROPERTY(bool isMuted READ isMuted WRITE setMuted NOTIFY isMutedChanged)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    /**
     * @brief Constructor that initializes media players.
     * @param parent Optional parent QObject.
     */
    explicit AudioController(QObject* parent = nullptr);

    ~AudioController() override = default;

    [[nodiscard]] auto isMuted() const noexcept -> bool;
    void setMuted(bool muted);
    
    [[nodiscard]] auto volume() const noexcept -> float;
    void setVolume(float volume);

    Q_INVOKABLE void toggleMute();

    /** @brief Play a crisp click sound effect for task completion. */
    Q_INVOKABLE void playClickSound();

public slots:
    /**
     * @brief Slot triggered when the timer state transitions.
     * @param newState The new timer state.
     * @param oldState The previous timer state.
     */
    void onTimerStateChanged(brain::domain::TimerState newState, brain::domain::TimerState oldState);

signals:
    void isMutedChanged(bool isMuted);
    void volumeChanged(float volume);

private:
    /** @brief Perform a 2-second audio crossfade from current to new source. */
    void crossfadeTo(const QString& newSource);

    QMediaPlayer* m_bgPlayer;
    QAudioOutput* m_bgOutput;
    
    QMediaPlayer* m_bellPlayer;
    QAudioOutput* m_bellOutput;

    QSoundEffect* m_clickSound;

    // Crossfade support
    QTimer*       m_fadeTimer;
    float         m_fadeTargetVolume{0.5f};
    float         m_fadeCurrentVolume{0.5f};
    QString       m_pendingSource;

    bool m_isMuted{false};       ///< Tracks the mute state.
    float m_volume{0.3f};        ///< Tracks the background volume level.
};

} // namespace brain::presentation

#endif // BRAIN_MAINTENANCE_PRESENTATION_AUDIOCONTROLLER_H
