/**
 * @file TimerViewModel.cpp
 * @brief Implementation of the MVVM TimerViewModel with configurable durations.
 *
 * Implements the focus timer state machine:
 *   Idle → Focusing → Overtime/CoolDown → Idle
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "TimerViewModel.h"
#include "infrastructure/AppConfig.h"

#include <QTimeZone>
#include <stdexcept>

namespace brain::presentation {

using brain::domain::TimerState;
using brain::infrastructure::AppConfig;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

TimerViewModel::TimerViewModel(
    std::shared_ptr<brain::domain::INoteSync> noteSync,
    QObject* parent)
    : QObject{parent}
    , m_state{TimerState::Idle}
    , m_remainingSeconds{0}
    , m_focusDurationMinutes{0}
    , m_coolDownDurationMinutes{0}
    , m_tickTimer{new QTimer{this}}
    , m_noteSync{std::move(noteSync)}
    , m_testOverride{false}
{
    if (!m_noteSync) {
        throw std::invalid_argument(
            "TimerViewModel: noteSync must not be null");
    }

    const auto& config = AppConfig::instance();
    m_focusDurationMinutes    = config.focusDurationMinutes();
    m_coolDownDurationMinutes = config.coolDownDurationMinutes();

    m_remainingSeconds = m_focusDurationMinutes * 60;

    m_tickTimer->setInterval(1000);
    connect(m_tickTimer, &QTimer::timeout,
            this, &TimerViewModel::onTimerTick);
}

// ---------------------------------------------------------------------------
// Property Accessors
// ---------------------------------------------------------------------------

auto TimerViewModel::remainingSeconds() const noexcept -> int {
    return m_remainingSeconds;
}

auto TimerViewModel::currentStateName() const -> QString {
    return stateToString(m_state);
}

auto TimerViewModel::timerState() const noexcept -> TimerState {
    return m_state;
}

auto TimerViewModel::focusDurationMinutes() const noexcept -> int {
    return m_focusDurationMinutes;
}

auto TimerViewModel::coolDownDurationMinutes() const noexcept -> int {
    return m_coolDownDurationMinutes;
}

auto TimerViewModel::sessionsCompletedToday() const noexcept -> int {
    return m_sessionsCompletedToday;
}

auto TimerViewModel::isOvertime() const noexcept -> bool {
    return m_isOvertime;
}

auto TimerViewModel::overtimeSeconds() const noexcept -> int {
    return m_overtimeSeconds;
}

// ---------------------------------------------------------------------------
// Property Mutators
// ---------------------------------------------------------------------------

void TimerViewModel::setFocusDurationMinutes(int minutes) {
    if (minutes <= 0) { return; }
    if (m_focusDurationMinutes == minutes) { return; }

    m_focusDurationMinutes = minutes;

    auto& config = AppConfig::instance();
    config.setFocusDurationMinutes(static_cast<std::int32_t>(minutes));

    emit focusDurationMinutesChanged(minutes);

    if (m_state == TimerState::Idle) {
        m_remainingSeconds = minutes * 60;
        emit remainingSecondsChanged(m_remainingSeconds);
    }
}

void TimerViewModel::setCoolDownDurationMinutes(int minutes) {
    if (minutes <= 0) { return; }
    if (m_coolDownDurationMinutes == minutes) { return; }

    m_coolDownDurationMinutes = minutes;

    auto& config = AppConfig::instance();
    config.setCoolDownDurationMinutes(static_cast<std::int32_t>(minutes));

    emit coolDownDurationMinutesChanged(minutes);
}

// ---------------------------------------------------------------------------
// Q_INVOKABLE Commands
// ---------------------------------------------------------------------------

void TimerViewModel::startFocus() {
    if (m_state == TimerState::Focusing) {
        return;
    }

    // Reset countdown unless a test override was injected
    if (!m_testOverride) {
        m_remainingSeconds = m_focusDurationMinutes * 60;
        emit remainingSecondsChanged(m_remainingSeconds);
    }
    m_testOverride = false;

    // Reset overtime state
    m_isOvertime = false;
    m_overtimeSeconds = 0;
    emit isOvertimeChanged(false);
    emit overtimeSecondsChanged(0);

    // Record focus start time in Taiwan timezone
    m_focusStartTime = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Taipei"));

    setState(TimerState::Focusing);
    m_tickTimer->start();
}

void TimerViewModel::pauseFocus() {
    if (m_state != TimerState::Focusing && m_state != TimerState::Overtime) {
        return;
    }

    m_tickTimer->stop();
    setState(TimerState::Paused);
}

void TimerViewModel::stopFocus() {
    if (m_state == TimerState::Idle) {
        return;
    }

    m_tickTimer->stop();
    m_isOvertime = false;
    m_overtimeSeconds = 0;
    emit isOvertimeChanged(false);
    emit overtimeSecondsChanged(0);
    m_remainingSeconds = m_focusDurationMinutes * 60;
    emit remainingSecondsChanged(m_remainingSeconds);
    setState(TimerState::Idle);
}

void TimerViewModel::startCoolDown() {
    // Allow starting break directly from Idle
    m_tickTimer->stop();
    m_isOvertime = false;
    m_overtimeSeconds = 0;
    emit isOvertimeChanged(false);
    emit overtimeSecondsChanged(0);
    m_pausedDuringCoolDown = false;

    m_remainingSeconds = m_coolDownDurationMinutes * 60;
    emit remainingSecondsChanged(m_remainingSeconds);

    setState(TimerState::CoolDown);
    m_tickTimer->start();
}

void TimerViewModel::finishFocusEarly() {
    if (m_state != TimerState::Focusing && m_state != TimerState::Overtime && m_state != TimerState::Paused) {
        return;
    }

    m_tickTimer->stop();

    // Emit session review signal so the UI can show the review popup
    emit sessionReviewRequested();
}

void TimerViewModel::submitSessionReview(const QString& text) {
    // Calculate time span in Taiwan timezone
    QDateTime endTime = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Taipei"));
    QString startStr = m_focusStartTime.toString("HH:mm");
    QString endStr = endTime.toString("HH:mm");

    // Calculate actual minutes elapsed
    qint64 elapsedSecs = m_focusStartTime.secsTo(endTime);
    int elapsedMins = static_cast<int>(elapsedSecs / 60);

    // Build the log entry
    QString logEntry;
    if (text.trimmed().isEmpty()) {
        logEntry = QString("**[%1 - %2 (%3m)]** (No review provided)")
                       .arg(startStr, endStr, QString::number(elapsedMins));
    } else {
        logEntry = QString("**[%1 - %2 (%3m)]** %4")
                       .arg(startStr, endStr, QString::number(elapsedMins), text.trimmed());
    }

    static_cast<void>(m_noteSync->syncText(logEntry.toStdString()));

    // Increment session count
    m_sessionsCompletedToday++;
    emit sessionsCompletedTodayChanged(m_sessionsCompletedToday);
    emit focusSessionCompleted();

    // Now transition to CoolDown
    m_isOvertime = false;
    m_overtimeSeconds = 0;
    emit isOvertimeChanged(false);
    emit overtimeSecondsChanged(0);

    m_remainingSeconds = m_coolDownDurationMinutes * 60;
    emit remainingSecondsChanged(m_remainingSeconds);
    setState(TimerState::CoolDown);
    m_tickTimer->start();
}

void TimerViewModel::adjustTime(int deltaMinutes) {
    if (m_state == TimerState::Idle) {
        int newMins = m_focusDurationMinutes + deltaMinutes;
        if (newMins < 1) newMins = 1;
        setFocusDurationMinutes(newMins);
        m_remainingSeconds = m_focusDurationMinutes * 60;
        emit remainingSecondsChanged(m_remainingSeconds);
    } else if (m_state == TimerState::Focusing || m_state == TimerState::Paused || m_state == TimerState::CoolDown) {
        int newTime = m_remainingSeconds + (deltaMinutes * 60);
        if (newTime < 60) newTime = 60; // Minimum 1 minute
        m_remainingSeconds = newTime;
        emit remainingSecondsChanged(m_remainingSeconds);
    }
}

void TimerViewModel::pauseCoolDown() {
    if (m_state != TimerState::CoolDown) return;
    m_tickTimer->stop();
    m_pausedDuringCoolDown = true;
    setState(TimerState::Paused);
}

void TimerViewModel::resumeCoolDown() {
    if (m_state != TimerState::Paused || !m_pausedDuringCoolDown) return;
    m_pausedDuringCoolDown = false;
    setState(TimerState::CoolDown);
    m_tickTimer->start();
}

void TimerViewModel::resume() {
    if (m_state != TimerState::Paused) return;

    if (m_pausedDuringCoolDown) {
        m_pausedDuringCoolDown = false;
        setState(TimerState::CoolDown);
    } else if (m_isOvertime) {
        setState(TimerState::Overtime);
    } else {
        setState(TimerState::Focusing);
    }
    
    m_tickTimer->start();
}

void TimerViewModel::submitTodo(const QString& text) {
    if (text.trimmed().isEmpty()) return;
    
    brain::domain::INoteSync::TaskItem task;
    task.text = text.toStdString();
    task.isCompleted = false;
    task.priority = 0;
    
    m_noteSync->appendTodo(task);
}

void TimerViewModel::submitTodoWithMetadata(const QString& text, int priority, const QString& dueDate) {
    if (text.trimmed().isEmpty()) return;
    
    brain::domain::INoteSync::TaskItem task;
    task.text = text.toStdString();
    task.isCompleted = false;
    task.priority = priority;
    task.dueDate = dueDate.toStdString();
    
    m_noteSync->appendTodo(task);
}

// ---------------------------------------------------------------------------
// Test Support
// ---------------------------------------------------------------------------

void TimerViewModel::setRemainingSecondsForTesting(int seconds) {
    m_remainingSeconds = seconds;
    m_testOverride = true;
    emit remainingSecondsChanged(m_remainingSeconds);
}

// ---------------------------------------------------------------------------
// Private Slots
// ---------------------------------------------------------------------------

void TimerViewModel::onTimerTick() {
    if (m_state == TimerState::Overtime) {
        // In overtime, count UP
        m_overtimeSeconds++;
        emit overtimeSecondsChanged(m_overtimeSeconds);
        return;
    }

    if (m_remainingSeconds <= 0) {
        m_tickTimer->stop();
        return;
    }

    --m_remainingSeconds;
    emit remainingSecondsChanged(m_remainingSeconds);

    if (m_remainingSeconds == 0) {
        m_tickTimer->stop();

        if (m_state == TimerState::Focusing) {
            // ── Focus session completed → enter Overtime (Flow State) ──
            // Don't force transition to CoolDown. Instead, enter Overtime
            // and let the user decide when to stop.
            m_isOvertime = true;
            m_overtimeSeconds = 0;
            emit isOvertimeChanged(true);
            emit overtimeSecondsChanged(0);

            setState(TimerState::Overtime);
            m_tickTimer->start(); // Keep ticking for overtime counter

            // Request session review (the UI will show it non-intrusively)
            emit sessionReviewRequested();

        } else if (m_state == TimerState::CoolDown) {
            // ── Cooldown completed ──
            m_remainingSeconds = m_focusDurationMinutes * 60;
            emit remainingSecondsChanged(m_remainingSeconds);

            setState(TimerState::Idle);
            emit coolDownCompleted();
        }
    }
}

// ---------------------------------------------------------------------------
// Private Helpers
// ---------------------------------------------------------------------------

auto TimerViewModel::stateToString(TimerState state) -> QString {
    switch (state) {
    case TimerState::Idle:     return QStringLiteral("Idle");
    case TimerState::Focusing: return QStringLiteral("Focusing");
    case TimerState::CoolDown: return QStringLiteral("CoolDown");
    case TimerState::Paused:   return QStringLiteral("Paused");
    case TimerState::Overtime: return QStringLiteral("Overtime");
    }
    return QStringLiteral("Unknown");
}

void TimerViewModel::setState(TimerState newState) {
    if (m_state == newState) {
        return;
    }
    TimerState oldState = m_state;
    m_state = newState;
    emit currentStateNameChanged(stateToString(m_state));
    emit timerStateChanged(m_state, oldState);
}

auto TimerViewModel::taiwanTimeString() const -> std::string {
    QDateTime tw = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Taipei"));
    return tw.toString("yyyy-MM-ddTHH:mm:ss").toStdString();
}

} // namespace brain::presentation
