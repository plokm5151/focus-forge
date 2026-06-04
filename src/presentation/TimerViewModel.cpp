/**
 * @file TimerViewModel.cpp
 * @brief Implementation of the MVVM TimerViewModel with configurable durations.
 *
 * Implements the focus timer state machine:
 *   Idle → Focusing → CoolDown → Idle
 *
 * Durations are sourced from AppConfig (persistent JSON) and the CoolDown
 * phase has its own configurable countdown.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "TimerViewModel.h"
#include "infrastructure/AppConfig.h"

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
    , m_tickTimer{new QTimer{this}}  // RAII: parent ownership prevents leaks
    , m_noteSync{std::move(noteSync)}
    , m_testOverride{false}
{
    if (!m_noteSync) {
        throw std::invalid_argument(
            "TimerViewModel: noteSync must not be null");
    }

    // Load durations from persistent config
    const auto& config = AppConfig::instance();
    m_focusDurationMinutes    = config.focusDurationMinutes();
    m_coolDownDurationMinutes = config.coolDownDurationMinutes();

    // Initialize countdown to focus duration
    m_remainingSeconds = m_focusDurationMinutes * 60;

    // Configure the tick timer for 1-second intervals
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

// ---------------------------------------------------------------------------
// Property Mutators
// ---------------------------------------------------------------------------

void TimerViewModel::setFocusDurationMinutes(int minutes) {
    if (minutes <= 0) { return; }
    if (m_focusDurationMinutes == minutes) { return; }

    m_focusDurationMinutes = minutes;

    // Persist to config
    auto& config = AppConfig::instance();
    config.setFocusDurationMinutes(static_cast<std::int32_t>(minutes));

    emit focusDurationMinutesChanged(minutes);

    // If idle, update the displayed countdown to reflect the new duration
    if (m_state == TimerState::Idle) {
        m_remainingSeconds = minutes * 60;
        emit remainingSecondsChanged(m_remainingSeconds);
    }
}

void TimerViewModel::setCoolDownDurationMinutes(int minutes) {
    if (minutes <= 0) { return; }
    if (m_coolDownDurationMinutes == minutes) { return; }

    m_coolDownDurationMinutes = minutes;

    // Persist to config
    auto& config = AppConfig::instance();
    config.setCoolDownDurationMinutes(static_cast<std::int32_t>(minutes));

    emit coolDownDurationMinutesChanged(minutes);
}

// ---------------------------------------------------------------------------
// Q_INVOKABLE Commands
// ---------------------------------------------------------------------------

void TimerViewModel::startFocus() {
    if (m_state == TimerState::Focusing) {
        return; // Already running — no-op
    }

    if (m_state == TimerState::Paused) {
        setState(TimerState::Focusing);
        m_tickTimer->start();
        return;
    }

    // Reset countdown unless a test override was injected
    if (!m_testOverride) {
        m_remainingSeconds = m_focusDurationMinutes * 60;
        emit remainingSecondsChanged(m_remainingSeconds);
    }
    m_testOverride = false; // Consume the override

    setState(TimerState::Focusing);
    m_tickTimer->start();
}

void TimerViewModel::pauseFocus() {
    if (m_state != TimerState::Focusing) {
        return; // Only pause when actively focusing
    }

    m_tickTimer->stop();
    setState(TimerState::Paused);
}

void TimerViewModel::stopFocus() {
    if (m_state == TimerState::Idle) {
        return;
    }

    m_tickTimer->stop();
    m_remainingSeconds = m_focusDurationMinutes * 60;
    emit remainingSecondsChanged(m_remainingSeconds);
    setState(TimerState::Idle);
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
    if (m_remainingSeconds <= 0) {
        m_tickTimer->stop();
        return;
    }

    --m_remainingSeconds;
    emit remainingSecondsChanged(m_remainingSeconds);

    if (m_remainingSeconds == 0) {
        m_tickTimer->stop();

        if (m_state == TimerState::Focusing) {
            // ── Focus session completed ──
            // 1. Sync the completed session via the injected strategy
            static_cast<void>(m_noteSync->syncText("Focus session completed"));

            // 2. Load cooldown duration and start cooldown countdown
            m_remainingSeconds = m_coolDownDurationMinutes * 60;
            emit remainingSecondsChanged(m_remainingSeconds);

            // 3. Transition to CoolDown
            setState(TimerState::CoolDown);
            emit focusSessionCompleted();

            // 4. Start the cooldown timer
            m_tickTimer->start();

        } else if (m_state == TimerState::CoolDown) {
            // ── Cooldown completed ──
            // Reset to focus duration and return to Idle
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

} // namespace brain::presentation
