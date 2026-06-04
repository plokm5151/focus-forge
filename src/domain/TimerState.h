/**
 * @file TimerState.h
 * @brief Defines the domain-level State Pattern enumeration for the focus timer.
 *
 * This enum is part of the domain layer and is consumed by both the
 * presentation layer (TimerViewModel) and test infrastructure. Keeping it
 * in the domain layer ensures the state machine semantics are owned by the
 * core business logic, not the UI framework.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#ifndef BRAIN_MAINTENANCE_DOMAIN_TIMERSTATE_H
#define BRAIN_MAINTENANCE_DOMAIN_TIMERSTATE_H

#include <cstdint>

namespace brain::domain {

/**
 * @enum TimerState
 * @brief Represents the discrete states of the focus timer state machine.
 *
 * The timer follows a linear state progression:
 *
 * @verbatim
 *   ┌──────────────────────────────────────────┐
 *   │  Idle ──▶ Focusing ──▶ CoolDown ──▶ Idle │
 *   └──────────────────────────────────────────┘
 * @endverbatim
 *
 * @par State Descriptions
 * - **Idle**: No active session. Timer is at its initial value, ready to start.
 * - **Focusing**: A focus session is in progress. Timer is counting down.
 * - **CoolDown**: Focus session has completed. A sync operation is triggered
 *   and the user enters a cooldown/rest period.
 */
enum class TimerState : std::uint8_t {
    Idle     = 0,  ///< No active session; timer ready.
    Focusing = 1,  ///< Focus session in progress; counting down.
    CoolDown = 2,  ///< Session completed; cooldown / rest period.
    Paused   = 3   ///< Session is paused by the user.
};

} // namespace brain::domain

#endif // BRAIN_MAINTENANCE_DOMAIN_TIMERSTATE_H
