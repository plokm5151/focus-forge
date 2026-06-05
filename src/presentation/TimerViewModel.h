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
#include <QDateTime>

#include <memory>

namespace brain::presentation {

/**
 * @class TimerViewModel
 * @brief ViewModel implementing MVVM for the configurable focus timer.
 */
class TimerViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(int remainingSeconds
               READ remainingSeconds
               NOTIFY remainingSecondsChanged)

    Q_PROPERTY(QString currentStateName
               READ currentStateName
               NOTIFY currentStateNameChanged)

    Q_PROPERTY(int focusDurationMinutes
               READ focusDurationMinutes
               WRITE setFocusDurationMinutes
               NOTIFY focusDurationMinutesChanged)

    Q_PROPERTY(int coolDownDurationMinutes
               READ coolDownDurationMinutes
               WRITE setCoolDownDurationMinutes
               NOTIFY coolDownDurationMinutesChanged)

    /** @brief Number of completed focus sessions today. */
    Q_PROPERTY(int sessionsCompletedToday
               READ sessionsCompletedToday
               NOTIFY sessionsCompletedTodayChanged)

    /** @brief Whether the timer is in overtime (flow state). */
    Q_PROPERTY(bool isOvertime
               READ isOvertime
               NOTIFY isOvertimeChanged)

    /** @brief Elapsed overtime seconds (counts up from 0). */
    Q_PROPERTY(int overtimeSeconds
               READ overtimeSeconds
               NOTIFY overtimeSecondsChanged)

public:
    explicit TimerViewModel(std::shared_ptr<brain::domain::INoteSync> noteSync,
                            QObject* parent = nullptr);

    ~TimerViewModel() override = default;

    // --- Property Accessors ---

    [[nodiscard]] auto remainingSeconds() const noexcept -> int;
    [[nodiscard]] auto currentStateName() const -> QString;
    [[nodiscard]] auto timerState() const noexcept -> brain::domain::TimerState;
    [[nodiscard]] auto focusDurationMinutes() const noexcept -> int;
    [[nodiscard]] auto coolDownDurationMinutes() const noexcept -> int;
    [[nodiscard]] auto sessionsCompletedToday() const noexcept -> int;
    [[nodiscard]] auto isOvertime() const noexcept -> bool;
    [[nodiscard]] auto overtimeSeconds() const noexcept -> int;

    // --- Property Mutators ---

    void setFocusDurationMinutes(int minutes);
    void setCoolDownDurationMinutes(int minutes);

    // --- Q_INVOKABLE Commands (Called from QML) ---

    Q_INVOKABLE void startFocus();
    Q_INVOKABLE void pauseFocus();
    Q_INVOKABLE void stopFocus();
    Q_INVOKABLE void submitTodo(const QString& text);
    Q_INVOKABLE void submitTodoWithMetadata(const QString& text, int priority, const QString& dueDate);

    /** @brief Start cooldown directly (skip focus). */
    Q_INVOKABLE void startCoolDown();

    /** @brief Finish early: end focus and go to cooldown (triggers review). */
    Q_INVOKABLE void finishFocusEarly();

    /** @brief Submit session review text and transition to CoolDown. */
    Q_INVOKABLE void submitSessionReview(const QString& text);

    /** @brief Adjust remaining time by delta minutes (e.g., +5 or -5). */
    Q_INVOKABLE void adjustTime(int deltaMinutes);

    /** @brief Pause cooldown. */
    Q_INVOKABLE void pauseCoolDown();

    /** @brief Resume cooldown. */
    Q_INVOKABLE void resumeCoolDown();

    // --- Test Support ---

    void setRemainingSecondsForTesting(int seconds);

signals:
    void remainingSecondsChanged(int seconds);
    void currentStateNameChanged(const QString& stateName);
    void focusDurationMinutesChanged(int minutes);
    void coolDownDurationMinutesChanged(int minutes);
    void sessionsCompletedTodayChanged(int count);
    void isOvertimeChanged(bool overtime);
    void overtimeSecondsChanged(int seconds);

    /** @brief Emitted when a focus session completes (Focusing → Overtime/CoolDown). */
    void focusSessionCompleted();

    /** @brief Emitted when the cooldown period completes (CoolDown → Idle). */
    void coolDownCompleted();

    /** @brief Emitted when the timer state machine transitions. */
    void timerStateChanged(brain::domain::TimerState newState, brain::domain::TimerState oldState);

    /** @brief Emitted when a session review is needed (user should see the review popup). */
    void sessionReviewRequested();

private slots:
    void onTimerTick();

private:
    [[nodiscard]] static auto stateToString(brain::domain::TimerState state) -> QString;
    void setState(brain::domain::TimerState newState);
    /** @brief Get the current Taiwan time string in ISO format. */
    [[nodiscard]] auto taiwanTimeString() const -> std::string;

    brain::domain::TimerState                   m_state;
    int                                         m_remainingSeconds;
    int                                         m_focusDurationMinutes;
    int                                         m_coolDownDurationMinutes;
    QTimer*                                     m_tickTimer;    ///< Parented to `this` (RAII).
    std::shared_ptr<brain::domain::INoteSync>   m_noteSync;
    bool                                        m_testOverride;

    // --- New fields for 24-point UX overhaul ---
    int                                         m_sessionsCompletedToday{0};
    bool                                        m_isOvertime{false};
    int                                         m_overtimeSeconds{0};
    QDateTime                                   m_focusStartTime;  ///< When the current focus started (Taiwan time).
    bool                                        m_pausedDuringCoolDown{false};
};

} // namespace brain::presentation

#endif // BRAIN_MAINTENANCE_PRESENTATION_TIMERVIEWMODEL_H
