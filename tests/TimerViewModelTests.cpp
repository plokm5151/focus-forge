/**
 * @file TimerViewModelTests.cpp
 * @brief TDD unit tests for TimerViewModel using Google Test and Google Mock.
 *
 * These tests verify the core state machine behavior of the focus timer,
 * including initial state, state transitions, countdown completion,
 * and dependency injection verification via the mocked INoteSync interface.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "domain/INoteSync.h"
#include "domain/TimerState.h"
#include "presentation/TimerViewModel.h"

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>

#include <memory>
#include <string_view>

// ===========================================================================
// Mock: INoteSync
// ===========================================================================

/**
 * @class MockNoteSync
 * @brief GMock implementation of INoteSync for isolated unit testing.
 *
 * Allows verification that syncText() is called with expected arguments
 * and the correct number of times during state transitions.
 */
class MockNoteSync : public brain::domain::INoteSync {
public:
    MockNoteSync() = default;
    ~MockNoteSync() override = default;

    MOCK_METHOD(bool, syncText, (std::string_view text), (override));
};

// ===========================================================================
// Test Fixture
// ===========================================================================

/**
 * @class TimerViewModelTest
 * @brief Google Test fixture providing shared setup for TimerViewModel tests.
 *
 * Creates a QCoreApplication (required for Qt event loop / signal-slot),
 * a mock INoteSync, and injects it into a fresh TimerViewModel per test.
 */
class TimerViewModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // QCoreApplication requires argc/argv; provide minimal stubs
        if (QCoreApplication::instance() == nullptr) {
            static int    argc = 1;
            static char   appName[] = "NeuroDashTests";
            static char*  argv[]    = {appName, nullptr};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }

        m_mockSync = std::make_shared<MockNoteSync>();
        m_viewModel = std::make_unique<brain::presentation::TimerViewModel>(
            m_mockSync);
    }

    void TearDown() override {
        m_viewModel.reset();
        m_mockSync.reset();
    }

    /// @brief The shared mock sync dependency.
    std::shared_ptr<MockNoteSync> m_mockSync;

    /// @brief The ViewModel under test.
    std::unique_ptr<brain::presentation::TimerViewModel> m_viewModel;

private:
    /// @brief Qt application instance (if none exists yet).
    std::unique_ptr<QCoreApplication> m_app;
};

// ===========================================================================
// Test Cases
// ===========================================================================

/**
 * @test Verifies the initial state of the TimerViewModel.
 *
 * The timer must start at 90*60 = 5400 seconds with state "Idle".
 */
TEST_F(TimerViewModelTest, InitialState_Is5400SecondsAndIdle) {
    EXPECT_EQ(m_viewModel->remainingSeconds(), 5400)
        << "Initial remaining seconds must be 90 * 60 = 5400";

    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("Idle"))
        << "Initial state must be 'Idle'";
}

/**
 * @test Verifies that calling startFocus() transitions state to Focusing.
 *
 * The state name must change to "Focusing" and the remainingSeconds
 * should remain at 5400 (countdown hasn't ticked yet).
 */
TEST_F(TimerViewModelTest, StartFocus_TransitionsToFocusing) {
    // Arrange: set up signal spy for state change
    QSignalSpy stateSpy(m_viewModel.get(),
                        &brain::presentation::TimerViewModel::currentStateNameChanged);

    // Act
    m_viewModel->startFocus();

    // Assert
    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("Focusing"))
        << "State must transition to 'Focusing' after startFocus()";

    EXPECT_EQ(stateSpy.count(), 1)
        << "currentStateNameChanged must be emitted exactly once";

    EXPECT_EQ(m_viewModel->remainingSeconds(), 5400)
        << "Remaining seconds should still be 5400 before any tick";
}

/**
 * @test Verifies state transition to CoolDown when timer reaches zero.
 *
 * Simulates the timer ticking down to 0 by processing the Qt event loop
 * with a minimal remaining time, then asserts that:
 * - The state transitions to "CoolDown".
 * - The focusSessionCompleted signal is emitted.
 */
TEST_F(TimerViewModelTest, TimerReachesZero_TransitionsToCoolDown) {
    // Arrange: inject a very short remaining time for fast test execution
    m_viewModel->setRemainingSecondsForTesting(1);
    m_viewModel->startFocus();

    QSignalSpy completionSpy(m_viewModel.get(),
                             &brain::presentation::TimerViewModel::focusSessionCompleted);

    // Act: process the event loop until the timer fires (max 3 seconds timeout)
    EXPECT_TRUE(completionSpy.wait(3000))
        << "focusSessionCompleted signal must be emitted within timeout";

    // Assert
    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("CoolDown"))
        << "State must transition to 'CoolDown' when timer reaches 0";

    EXPECT_EQ(m_viewModel->remainingSeconds(), 0)
        << "Remaining seconds must be 0 after completion";
}

/**
 * @test Verifies that syncText is called EXACTLY ONCE on session completion.
 *
 * This is the critical DI verification test: when the focus timer reaches
 * zero and transitions to CoolDown, the injected INoteSync::syncText must
 * be invoked exactly one time. This confirms the dependency injection
 * pipeline is correctly wired and the ViewModel properly delegates to
 * the sync strategy.
 */
TEST_F(TimerViewModelTest, CoolDownTransition_CallsSyncTextExactlyOnce) {
    using ::testing::_;
    using ::testing::Return;

    // Arrange: expect syncText to be called exactly once with any argument
    EXPECT_CALL(*m_mockSync, syncText(_))
        .Times(1)
        .WillOnce(Return(true));

    // Inject minimal remaining time for fast test execution
    m_viewModel->setRemainingSecondsForTesting(1);
    m_viewModel->startFocus();

    QSignalSpy completionSpy(m_viewModel.get(),
                             &brain::presentation::TimerViewModel::focusSessionCompleted);

    // Act: wait for the session to complete
    ASSERT_TRUE(completionSpy.wait(3000))
        << "focusSessionCompleted signal must be emitted";

    // Assert: GMock automatically verifies EXPECT_CALL expectations
    // in the MockNoteSync destructor. Explicit verification here for clarity.
    ::testing::Mock::VerifyAndClearExpectations(m_mockSync.get());
}

/**
 * @test Verifies that remainingSecondsChanged signal is emitted on each tick.
 *
 * Sets the timer to 2 seconds, starts focus, and verifies that the signal
 * fires as the countdown progresses.
 */
TEST_F(TimerViewModelTest, Tick_EmitsRemainingSecondsChanged) {
    using ::testing::_;
    using ::testing::Return;

    EXPECT_CALL(*m_mockSync, syncText(_))
        .WillRepeatedly(Return(true));

    m_viewModel->setRemainingSecondsForTesting(2);
    m_viewModel->startFocus();

    QSignalSpy secondsSpy(m_viewModel.get(),
                          &brain::presentation::TimerViewModel::remainingSecondsChanged);

    QSignalSpy completionSpy(m_viewModel.get(),
                             &brain::presentation::TimerViewModel::focusSessionCompleted);

    // Wait for completion (2 ticks: 2→1, 1→0)
    ASSERT_TRUE(completionSpy.wait(5000))
        << "Session must complete within timeout";

    EXPECT_GE(secondsSpy.count(), 2)
        << "remainingSecondsChanged must be emitted at least twice for a 2-second timer";
}

/**
 * @test Verifies that pauseFocus() has no effect when not in Focusing state.
 */
TEST_F(TimerViewModelTest, PauseFocus_WhenIdle_HasNoEffect) {
    QSignalSpy stateSpy(m_viewModel.get(),
                        &brain::presentation::TimerViewModel::currentStateNameChanged);

    m_viewModel->pauseFocus();

    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("Idle"))
        << "State must remain 'Idle' when pauseFocus() is called in Idle state";

    EXPECT_EQ(stateSpy.count(), 0)
        << "No state change signal should be emitted";
}
