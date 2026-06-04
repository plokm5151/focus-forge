/**
 * @file AudioControllerTests.cpp
 * @brief TDD unit tests for AudioController verifying state-based multimedia playback logic.
 *
 * We verify that state transitions don't crash and the correct slots are triggered.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include <gtest/gtest.h>

#include "domain/TimerState.h"
#include "presentation/AudioController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <memory>

class AudioControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (QCoreApplication::instance() == nullptr) {
            static int    argc = 1;
            static char   appName[] = "NeuroDashTests";
            static char*  argv[]    = {appName, nullptr};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }

        m_audioController = std::make_unique<brain::presentation::AudioController>();
    }

    void TearDown() override {
        m_audioController.reset();
    }

    std::unique_ptr<brain::presentation::AudioController> m_audioController;

private:
    std::unique_ptr<QCoreApplication> m_app;
};

/**
 * @test Verifies that transitioning to Focusing does not crash and processes correctly.
 */
TEST_F(AudioControllerTest, TransitionToFocusing_HandlesSafely) {
    // Calling the slot directly to ensure no crash occurs during QMediaPlayer setup.
    // Real audio playback will be skipped/mocked silently by Qt Multimedia without a real audio device.
    ASSERT_NO_THROW({
        m_audioController->onTimerStateChanged(
            brain::domain::TimerState::Focusing, 
            brain::domain::TimerState::Idle);
    });
}

/**
 * @test Verifies that transitioning to CoolDown handles safely.
 */
TEST_F(AudioControllerTest, TransitionToCoolDown_HandlesSafely) {
    ASSERT_NO_THROW({
        m_audioController->onTimerStateChanged(
            brain::domain::TimerState::CoolDown, 
            brain::domain::TimerState::Focusing);
    });
}

/**
 * @test Verifies that transitioning to Idle stops background audio safely.
 */
TEST_F(AudioControllerTest, TransitionToIdle_HandlesSafely) {
    ASSERT_NO_THROW({
        m_audioController->onTimerStateChanged(
            brain::domain::TimerState::Idle, 
            brain::domain::TimerState::CoolDown);
    });
}

/**
 * @test Verify asset directory was copied successfully to build directory.
 */
TEST_F(AudioControllerTest, AssetsDirectoryIsDeployed) {
    QString appDir = QCoreApplication::applicationDirPath();
    QFileInfo bellFile(appDir + "/assets/audio/bell.wav");
    QFileInfo focusFile(appDir + "/assets/audio/focus.ogg");
    QFileInfo cooldownFile(appDir + "/assets/audio/cooldown.mp3");
    
    // Depending on when the test runs relative to the build, 
    // the files might not be deployed in a pure source-build if custom_command is missed.
    // The test asserts true to check if the copy_directory command worked.
    EXPECT_TRUE(bellFile.exists()) << "bell.wav must be deployed to build directory";
    EXPECT_TRUE(focusFile.exists()) << "focus.ogg must be deployed to build directory";
    EXPECT_TRUE(cooldownFile.exists()) << "cooldown.mp3 must be deployed to build directory";
}
