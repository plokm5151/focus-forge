/**
 * @file AudioController.h
 * @brief Manages multimedia playback for focus sessions.
 *
 * Implements audio playback for the dashboard using Qt6::Multimedia.
 * It manages two distinct types of audio:
 * - Looping background ambients (QMediaPlayer) for focus and cooldown.
 * - Zero-latency one-shot sound effects (QSoundEffect) for the transition bell.
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

public:
    /**
     * @brief Constructor that initializes media players.
     * @param parent Optional parent QObject.
     */
    explicit AudioController(QObject* parent = nullptr);

    ~AudioController() override = default;

    [[nodiscard]] auto isMuted() const noexcept -> bool;
    void setMuted(bool muted);
    
    Q_INVOKABLE void toggleMute();

public slots:
    /**
     * @brief Slot triggered when the timer state transitions.
     * @param newState The new timer state.
     * @param oldState The previous timer state.
     */
    void onTimerStateChanged(brain::domain::TimerState newState, brain::domain::TimerState oldState);

signals:
    void isMutedChanged(bool muted);

private:
    QMediaPlayer* m_bgPlayer;    ///< Plays background loop audio.
    QAudioOutput* m_bgOutput;    ///< Audio output for the media player.
    QMediaPlayer* m_bellPlayer;  ///< Plays the transition bell using QMediaPlayer.
    QAudioOutput* m_bellOutput;  ///< Audio output for the bell player.
    bool m_isMuted{false};       ///< Tracks the mute state.
};

} // namespace brain::presentation

#endif // BRAIN_MAINTENANCE_PRESENTATION_AUDIOCONTROLLER_H
