/**
 * @file TimerViewModelTests.cpp
 * @brief TDD unit tests for TimerViewModel using Google Test and Google Mock.
 *
 * Tests verify the configurable timer state machine including:
 * - Initial state with AppConfig-driven durations (40 min = 2400s)
 * - State transitions: Idle → Focusing → CoolDown → Idle
 * - CoolDown loads cooldownDuration (10 min = 600s)
 * - Dependency injection verification via mocked INoteSync
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

#include <memory>
#include <string_view>

// ===========================================================================
// Mock: INoteSync
// ===========================================================================

/**
 * @class MockNoteSync
 * @brief GMock implementation of INoteSync for isolated unit testing.
 */
class MockNoteSync : public brain::domain::INoteSync {
public:
    MockNoteSync() = default;
    ~MockNoteSync() override = default;

    MOCK_METHOD(bool, syncText, (std::string_view text), (override));
    MOCK_METHOD(void, appendTodo, (std::string_view taskText), (override));
    MOCK_METHOD(std::vector<TaskItem>, readTasks, (), (const, override));
    MOCK_METHOD(void, updateTask, (int index, bool isCompleted), (override));
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
 * AppConfig defaults (40 min focus, 10 min cooldown) are used since no
 * config.json exists in the test environment.
 */
class TimerViewModelTest : public ::testing::Test {
protected:
    void SetUp() override {
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

    std::shared_ptr<MockNoteSync> m_mockSync;
    std::unique_ptr<brain::presentation::TimerViewModel> m_viewModel;

private:
    std::unique_ptr<QCoreApplication> m_app;
};

// ===========================================================================
// Test Cases
// ===========================================================================

/**
 * @test Verifies the initial state of the TimerViewModel.
 *
 * With AppConfig defaults (40 min focus), the timer must start at
 * 40 * 60 = 2400 seconds with state "Idle".
 */
TEST_F(TimerViewModelTest, InitialState_Is2400SecondsAndIdle) {
    EXPECT_EQ(m_viewModel->remainingSeconds(), 40 * 60)
        << "Initial remaining seconds must be 40 * 60 = 2400 (from AppConfig)";

    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("Idle"))
        << "Initial state must be 'Idle'";

    EXPECT_EQ(m_viewModel->focusDurationMinutes(), 40)
        << "Focus duration must default to 40 minutes";

    EXPECT_EQ(m_viewModel->coolDownDurationMinutes(), 10)
        << "CoolDown duration must default to 10 minutes";
}

/**
 * @test Verifies that calling startFocus() transitions state to Focusing.
 */
TEST_F(TimerViewModelTest, StartFocus_TransitionsToFocusing) {
    QSignalSpy stateSpy(m_viewModel.get(),
                        &brain::presentation::TimerViewModel::currentStateNameChanged);

    m_viewModel->startFocus();

    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("Focusing"))
        << "State must transition to 'Focusing' after startFocus()";

    EXPECT_EQ(stateSpy.count(), 1)
        << "currentStateNameChanged must be emitted exactly once";
}

/**
 * @test Verifies that when the focus timer reaches zero, the state
 * transitions to CoolDown and the remaining seconds are loaded with
 * the cooldown duration (10 * 60 = 600 seconds).
 */
TEST_F(TimerViewModelTest, FocusCompletion_TransitionsToCoolDownWithCorrectDuration) {
    using ::testing::_;
    using ::testing::Return;

    EXPECT_CALL(*m_mockSync, syncText(_))
        .WillOnce(Return(true));

    // Inject short remaining time for fast test execution
    m_viewModel->setRemainingSecondsForTesting(1);
    m_viewModel->startFocus();

    QSignalSpy completionSpy(m_viewModel.get(),
                             &brain::presentation::TimerViewModel::focusSessionCompleted);

    ASSERT_TRUE(completionSpy.wait(3000))
        << "focusSessionCompleted signal must be emitted within timeout";

    // Verify state transition to CoolDown
    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("CoolDown"))
        << "State must transition to 'CoolDown' when focus timer reaches 0";

    // Verify CoolDown duration is loaded from config (10 min = 600s)
    EXPECT_EQ(m_viewModel->remainingSeconds(), 10 * 60)
        << "CoolDown must load coolDownDurationMinutes (default: 10 min = 600s)";
}

/**
 * @test Verifies that syncText is called EXACTLY ONCE on focus completion.
 *
 * Critical DI verification: when transitioning from Focusing to CoolDown,
 * the injected INoteSync::syncText must be invoked exactly one time.
 */
TEST_F(TimerViewModelTest, CoolDownTransition_CallsSyncTextExactlyOnce) {
    using ::testing::_;
    using ::testing::Return;

    EXPECT_CALL(*m_mockSync, syncText(_))
        .Times(1)
        .WillOnce(Return(true));

    m_viewModel->setRemainingSecondsForTesting(1);
    m_viewModel->startFocus();

    QSignalSpy completionSpy(m_viewModel.get(),
                             &brain::presentation::TimerViewModel::focusSessionCompleted);

    ASSERT_TRUE(completionSpy.wait(3000))
        << "focusSessionCompleted signal must be emitted";

    // GMock automatically verifies EXPECT_CALL expectations
    ::testing::Mock::VerifyAndClearExpectations(m_mockSync.get());
}

/**
 * @test Verifies the full cycle: Focusing → CoolDown → Idle.
 *
 * After focus completes, the cooldown timer counts down. When cooldown
 * reaches zero, the state returns to Idle with focus duration reloaded.
 */
TEST_F(TimerViewModelTest, FullCycle_FocusingToCoolDownToIdle) {
    using ::testing::_;
    using ::testing::Return;

    EXPECT_CALL(*m_mockSync, syncText(_))
        .WillOnce(Return(true));

    // Start with 1 second of focus
    m_viewModel->setRemainingSecondsForTesting(1);
    m_viewModel->startFocus();

    QSignalSpy completionSpy(m_viewModel.get(),
                             &brain::presentation::TimerViewModel::focusSessionCompleted);

    ASSERT_TRUE(completionSpy.wait(3000));
    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("CoolDown"));

    // Now inject 1 second of cooldown to expedite
    m_viewModel->setRemainingSecondsForTesting(1);

    QSignalSpy coolDownSpy(m_viewModel.get(),
                           &brain::presentation::TimerViewModel::coolDownCompleted);

    ASSERT_TRUE(coolDownSpy.wait(3000))
        << "coolDownCompleted signal must be emitted";

    // Verify return to Idle with focus duration reloaded
    EXPECT_EQ(m_viewModel->currentStateName(), QStringLiteral("Idle"))
        << "State must return to 'Idle' after cooldown completes";

    EXPECT_EQ(m_viewModel->remainingSeconds(), 40 * 60)
        << "Remaining seconds must be reset to focus duration (2400s)";
}

/**
 * @test Verifies that remainingSecondsChanged signal is emitted on each tick.
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

    ASSERT_TRUE(completionSpy.wait(5000))
        << "Session must complete within timeout";

    // 2 ticks (2→1, 1→0) + 1 for CoolDown loading = at least 3 emissions
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

/**
 * @test Verifies that duration properties can be read correctly.
 */
TEST_F(TimerViewModelTest, DurationProperties_MatchAppConfigDefaults) {
    EXPECT_EQ(m_viewModel->focusDurationMinutes(), 40);
    EXPECT_EQ(m_viewModel->coolDownDurationMinutes(), 10);
}

/**
 * @test Verifies that submitTodo routes correctly to the injected INoteSync.
 */
TEST_F(TimerViewModelTest, SubmitTodo_CallsAppendTodo) {
    using ::testing::_;
    using ::testing::Eq;

    EXPECT_CALL(*m_mockSync, appendTodo(Eq("Test task")))
        .Times(1);

    m_viewModel->submitTodo("Test task");
}
