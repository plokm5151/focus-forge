/**
 * @file TimerViewModel.h
 * @brief MVVM ViewModel for the focus timer, bridging domain logic with QML.
 *
 * Manages the focus timer state machine and exposes reactive properties
 * to the QML presentation layer via Qt's property system. Durations are
 * fetched from AppConfig for full configurability.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#ifndef BRAIN_MAINTENANCE_PRESENTATION_TIMERVIEWMODEL_H
#define BRAIN_MAINTENANCE_PRESENTATION_TIMERVIEWMODEL_H

#include "domain/INoteSync.h"
#include "domain/TimerState.h"

#include <QObject>
#include <QString>
#include <QTimer>

#include <memory>

namespace brain::presentation {

/**
 * @class TimerViewModel
 * @brief ViewModel implementing MVVM for the configurable focus timer.
 *
 * Owns the timer state machine and exposes it to QML through Q_PROPERTY
 * bindings. Durations are read from AppConfig, enabling runtime and
 * persistent configuration. Dependencies (e.g., note sync strategy) are
 * injected via constructor.
 *
 * @par State Machine
 * @verbatim
 *   Idle ──[startFocus()]──▶ Focusing ──[timer=0]──▶ CoolDown ──[timer=0]──▶ Idle
 * @endverbatim
 *
 * @par Configuration
 * - Focus duration: `AppConfig::focusDurationMinutes()` (default 40 min)
 * - CoolDown duration: `AppConfig::coolDownDurationMinutes()` (default 10 min)
 *
 * @par Thread Safety
 * Must be used exclusively from the Qt main thread (GUI thread).
 *
 * @see brain::domain::INoteSync
 * @see brain::domain::TimerState
 * @see brain::infrastructure::AppConfig
 */
class TimerViewModel : public QObject {
    Q_OBJECT

    /**
     * @property remainingSeconds
     * @brief Seconds remaining in the current session (focus or cooldown).
     */
    Q_PROPERTY(int remainingSeconds
               READ remainingSeconds
               NOTIFY remainingSecondsChanged)

    /**
     * @property currentStateName
     * @brief Human-readable name of the current timer state.
     *
     * Possible values: "Idle", "Focusing", "CoolDown".
     */
    Q_PROPERTY(QString currentStateName
               READ currentStateName
               NOTIFY currentStateNameChanged)

    /**
     * @property focusDurationMinutes
     * @brief Configurable focus session duration in minutes.
     *
     * Read from AppConfig on initialization. Can be modified at runtime
     * via QML or programmatically; changes are persisted to config.json.
     */
    Q_PROPERTY(int focusDurationMinutes
               READ focusDurationMinutes
               WRITE setFocusDurationMinutes
               NOTIFY focusDurationMinutesChanged)

    /**
     * @property coolDownDurationMinutes
     * @brief Configurable cooldown (break) duration in minutes.
     *
     * Read from AppConfig on initialization. Can be modified at runtime
     * via QML or programmatically; changes are persisted to config.json.
     */
    Q_PROPERTY(int coolDownDurationMinutes
               READ coolDownDurationMinutes
               WRITE setCoolDownDurationMinutes
               NOTIFY coolDownDurationMinutesChanged)

public:
    /**
     * @brief Constructs the ViewModel with an injected sync dependency.
     *
     * Reads initial durations from AppConfig. Sets remainingSeconds to
     * focusDurationMinutes * 60.
     *
     * @param noteSync Shared pointer to a concrete INoteSync implementation.
     *                 Must not be null.
     * @param parent   Optional QObject parent for Qt's ownership tree.
     *
     * @throws std::invalid_argument if noteSync is null.
     */
    explicit TimerViewModel(std::shared_ptr<brain::domain::INoteSync> noteSync,
                            QObject* parent = nullptr);

    ~TimerViewModel() override = default;

    // --- Property Accessors ---

    [[nodiscard]] auto remainingSeconds() const noexcept -> int;
    [[nodiscard]] auto currentStateName() const -> QString;
    [[nodiscard]] auto timerState() const noexcept -> brain::domain::TimerState;
    [[nodiscard]] auto focusDurationMinutes() const noexcept -> int;
    [[nodiscard]] auto coolDownDurationMinutes() const noexcept -> int;

    // --- Property Mutators ---

    /**
     * @brief Sets focus duration and persists to AppConfig.
     * @param minutes Duration in minutes. Must be > 0.
     */
    void setFocusDurationMinutes(int minutes);

    /**
     * @brief Sets cooldown duration and persists to AppConfig.
     * @param minutes Duration in minutes. Must be > 0.
     */
    void setCoolDownDurationMinutes(int minutes);

    // --- Q_INVOKABLE Commands (Called from QML) ---

    /**
     * @brief Starts a new focus session.
     *
     * Resets the countdown to focusDurationMinutes * 60 and transitions
     * to the Focusing state. No-op if already Focusing.
     */
    Q_INVOKABLE void startFocus();

    /**
     * @brief Pauses the currently running focus session.
     *
     * Stops the countdown timer. No-op if not in Focusing state.
     */
    Q_INVOKABLE void pauseFocus();

    /**
     * @brief Submits a new task to the note sync strategy.
     * @param text The task description.
     */
    Q_INVOKABLE void submitTodo(const QString& text);

    // --- Test Support ---

    /**
     * @brief Sets remaining seconds directly (for testing only).
     *
     * Allows tests to inject a short duration to avoid waiting the full
     * configured duration. Must be called before startFocus().
     *
     * @param seconds The number of seconds to set.
     */
    void setRemainingSecondsForTesting(int seconds);

signals:
    void remainingSecondsChanged(int seconds);
    void currentStateNameChanged(const QString& stateName);
    void focusDurationMinutesChanged(int minutes);
    void coolDownDurationMinutesChanged(int minutes);

    /**
     * @brief Emitted when a focus session completes (Focusing → CoolDown).
     */
    void focusSessionCompleted();

    /**
     * @brief Emitted when the cooldown period completes (CoolDown → Idle).
     */
    void coolDownCompleted();

    /**
     * @brief Emitted when the timer state machine transitions.
     * @param newState The new TimerState.
     * @param oldState The previous TimerState.
     */
    void timerStateChanged(brain::domain::TimerState newState, brain::domain::TimerState oldState);

private slots:
    /**
     * @brief Handles each tick of the internal QTimer.
     *
     * Decrements remaining seconds. On reaching zero:
     * - In Focusing: syncs, transitions to CoolDown, starts cooldown timer.
     * - In CoolDown: transitions to Idle, resets to focus duration.
     */
    void onTimerTick();

private:
    [[nodiscard]] static auto stateToString(brain::domain::TimerState state) -> QString;
    void setState(brain::domain::TimerState newState);

    brain::domain::TimerState                   m_state;
    int                                         m_remainingSeconds;
    int                                         m_focusDurationMinutes;
    int                                         m_coolDownDurationMinutes;
    QTimer*                                     m_tickTimer;    ///< Parented to `this` (RAII).
    std::shared_ptr<brain::domain::INoteSync>   m_noteSync;
    bool                                        m_testOverride;
};

} // namespace brain::presentation

#endif // BRAIN_MAINTENANCE_PRESENTATION_TIMERVIEWMODEL_H
