/**
 * @file TimerViewModel.cpp
 * @brief Implementation of the MVVM TimerViewModel.
 *
 * Implements the focus timer state machine with transitions:
 *   Idle → Focusing → CoolDown
 *
 * Uses QTimer (parented to `this`) for RAII-compliant countdown and
 * delegates session completion logging to the injected INoteSync strategy.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "TimerViewModel.h"

#include <stdexcept>

namespace brain::presentation {

using brain::domain::TimerState;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

TimerViewModel::TimerViewModel(
    std::shared_ptr<brain::domain::INoteSync> noteSync,
    QObject* parent)
    : QObject{parent}
    , m_state{TimerState::Idle}
    , m_remainingSeconds{kDefaultFocusDurationSeconds}
    , m_tickTimer{new QTimer{this}}  // RAII: parent ownership prevents leaks
    , m_noteSync{std::move(noteSync)}
    , m_testOverride{false}
{
    if (!m_noteSync) {
        throw std::invalid_argument(
            "TimerViewModel: noteSync must not be null");
    }

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

// ---------------------------------------------------------------------------
// Q_INVOKABLE Commands
// ---------------------------------------------------------------------------

void TimerViewModel::startFocus() {
    if (m_state == TimerState::Focusing) {
        return; // Already running — no-op
    }

    // Reset remaining seconds unless a test override was set
    if (!m_testOverride) {
        m_remainingSeconds = kDefaultFocusDurationSeconds;
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
    // Remain in Focusing state but timer is stopped
    // (future enhancement: add a Paused state if needed)
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

        // Log the completed session via the injected sync strategy
        m_noteSync->syncText("Focus session completed");

        // Transition to CoolDown state
        setState(TimerState::CoolDown);

        emit focusSessionCompleted();
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
    }
    return QStringLiteral("Unknown");
}

void TimerViewModel::setState(TimerState newState) {
    if (m_state == newState) {
        return;
    }
    m_state = newState;
    emit currentStateNameChanged(stateToString(m_state));
}

} // namespace brain::presentation
