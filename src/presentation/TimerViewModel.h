/**
 * @file TimerViewModel.h
 * @brief MVVM ViewModel for the focus timer, bridging domain logic with QML.
 *
 * Manages the focus timer state machine and exposes reactive properties
 * to the QML presentation layer via Qt's property system. Uses the
 * domain-level TimerState enum for state representation.
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
 * @brief Default focus session duration: 90 minutes in seconds.
 */
inline constexpr int kDefaultFocusDurationSeconds = 90 * 60; // 5400

/**
 * @class TimerViewModel
 * @brief ViewModel implementing MVVM for the focus timer.
 *
 * Owns the timer state machine and exposes it to QML through Q_PROPERTY
 * bindings. Dependencies (e.g., note sync strategy) are injected via
 * constructor, following the Dependency Injection principle.
 *
 * @par State Machine
 * @verbatim
 *   Idle ──[startFocus()]──▶ Focusing ──[timer=0]──▶ CoolDown
 *                                ▲           │
 *                                └──[resume]──┘
 *                                  (paused)
 * @endverbatim
 *
 * @par MVVM Architecture
 * - **Model**: Timer state (remaining seconds, current state name).
 * - **ViewModel**: This class — transforms model state into QML-bindable
 *   properties and commands.
 * - **View**: QML UI (bound via `setContextProperty`).
 *
 * @par Dependency Injection
 * The constructor requires a `std::shared_ptr<INoteSync>`, ensuring that
 * the ViewModel never creates its own dependencies and remains testable.
 *
 * @par Thread Safety
 * Must be used exclusively from the Qt main thread (GUI thread).
 *
 * @see brain::domain::INoteSync
 * @see brain::domain::TimerState
 */
class TimerViewModel : public QObject {
    Q_OBJECT

    /**
     * @property remainingSeconds
     * @brief Seconds remaining in the current focus session.
     *
     * Updated every second while the timer is running. QML views should
     * bind to this property for real-time countdown display.
     * Initial value: 5400 (90 minutes).
     */
    Q_PROPERTY(int remainingSeconds
               READ remainingSeconds
               NOTIFY remainingSecondsChanged)

    /**
     * @property currentStateName
     * @brief Human-readable name of the current timer state.
     *
     * Possible values: "Idle", "Focusing", "CoolDown".
     * QML views can use this to update UI labels and color schemes.
     */
    Q_PROPERTY(QString currentStateName
               READ currentStateName
               NOTIFY currentStateNameChanged)

public:
    /**
     * @brief Constructs the ViewModel with an injected sync dependency.
     *
     * @param noteSync Shared pointer to a concrete INoteSync implementation.
     *                 Must not be null.
     * @param parent   Optional QObject parent for Qt's ownership tree.
     *
     * @throws std::invalid_argument if noteSync is null.
     */
    explicit TimerViewModel(std::shared_ptr<brain::domain::INoteSync> noteSync,
                            QObject* parent = nullptr);

    /**
     * @brief Default destructor.
     */
    ~TimerViewModel() override = default;

    // --- Property Accessors ---

    /**
     * @brief Returns the remaining seconds in the current session.
     * @return Seconds remaining (>= 0).
     */
    [[nodiscard]] auto remainingSeconds() const noexcept -> int;

    /**
     * @brief Returns the current state name.
     * @return State name string (e.g., "Focusing", "Idle", "CoolDown").
     */
    [[nodiscard]] auto currentStateName() const -> QString;

    /**
     * @brief Returns the current domain-level timer state.
     * @return The current TimerState enum value.
     */
    [[nodiscard]] auto timerState() const noexcept -> brain::domain::TimerState;

    // --- Q_INVOKABLE Commands (Called from QML) ---

    /**
     * @brief Starts or resumes a focus session.
     *
     * If the timer is idle or in cooldown, starts a new focus session
     * with the default duration (5400s). If paused, resumes countdown.
     */
    Q_INVOKABLE void startFocus();

    /**
     * @brief Pauses the currently running focus session.
     *
     * Has no effect if the timer is not in the "Focusing" state.
     */
    Q_INVOKABLE void pauseFocus();

    // --- Test Support ---

    /**
     * @brief Sets the remaining seconds directly (for testing only).
     *
     * Allows tests to inject a short duration to avoid waiting 90 minutes.
     * Must be called before startFocus() for the override to take effect.
     *
     * @param seconds The number of seconds to set.
     */
    void setRemainingSecondsForTesting(int seconds);

signals:
    /**
     * @brief Emitted when remainingSeconds changes.
     * @param seconds The updated remaining seconds value.
     */
    void remainingSecondsChanged(int seconds);

    /**
     * @brief Emitted when the timer state changes.
     * @param stateName The new state name string.
     */
    void currentStateNameChanged(const QString& stateName);

    /**
     * @brief Emitted when a focus session completes (timer reaches 0).
     */
    void focusSessionCompleted();

private slots:
    /**
     * @brief Handles each tick of the internal QTimer.
     *
     * Decrements remaining seconds, emits property change signals,
     * and triggers session completion logic when the timer reaches zero.
     */
    void onTimerTick();

private:
    /**
     * @brief Converts a TimerState enum to its display string.
     * @param state The timer state to convert.
     * @return Human-readable state name.
     */
    [[nodiscard]] static auto stateToString(brain::domain::TimerState state) -> QString;

    /**
     * @brief Sets the current state and emits the change signal.
     * @param newState The target state.
     */
    void setState(brain::domain::TimerState newState);

    brain::domain::TimerState                   m_state;            ///< Current timer state.
    int                                         m_remainingSeconds; ///< Countdown value.
    QTimer*                                     m_tickTimer;        ///< 1-second interval timer (owned via QObject parent).
    std::shared_ptr<brain::domain::INoteSync>   m_noteSync;         ///< Injected sync strategy.
    bool                                        m_testOverride;     ///< If true, startFocus() skips resetting remaining seconds.
};

} // namespace brain::presentation

#endif // BRAIN_MAINTENANCE_PRESENTATION_TIMERVIEWMODEL_H
