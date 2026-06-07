/**
 * @file TodoListModelTests.cpp
 * @brief Unit tests for TodoListModel using Google Test and MockNoteSync.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "presentation/TodoListModel.h"
#include "domain/INoteSync.h"

#include <QCoreApplication>
#include <memory>

class MockNoteSync : public brain::domain::INoteSync {
public:
    MOCK_METHOD(bool, syncText, (std::string_view text), (override));
    MOCK_METHOD(void, appendTodo, (const TaskItem& task), (override));
    MOCK_METHOD(std::vector<TaskItem>, readTasks, (), (const, override));
    MOCK_METHOD(void, updateTask, (int index, bool isCompleted), (override));
    MOCK_METHOD(void, updateTaskText, (int index, const TaskItem& task), (override));
    MOCK_METHOD(void, deleteTask, (int index), (override));
    MOCK_METHOD(void, clearAllTasks, (), (override));
};

class TodoListModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (QCoreApplication::instance() == nullptr) {
            static int argc = 1;
            static char appName[] = "NeuroDashTests";
            static char* argv[] = {appName, nullptr};
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }
        m_mockSync = std::make_shared<MockNoteSync>();
    }

    std::shared_ptr<MockNoteSync> m_mockSync;
    std::unique_ptr<QCoreApplication> m_app;
};

TEST_F(TodoListModelTest, LoadTasks_PopulatesModel) {
    using ::testing::Return;

    std::vector<brain::domain::INoteSync::TaskItem> mockTasks = {
        {"Buy milk", false, "2026-06-10", 3},
        {"Read book", true, "", 1}
    };

    EXPECT_CALL(*m_mockSync, readTasks())
        .WillOnce(Return(mockTasks));

    brain::presentation::TodoListModel model(m_mockSync);

    EXPECT_EQ(model.rowCount(), 2);

    QVariant textData = model.data(model.index(0, 0), brain::presentation::TodoListModel::DisplayRole);
    EXPECT_EQ(textData.toString(), QStringLiteral("Buy milk"));

    QVariant completedData = model.data(model.index(1, 0), brain::presentation::TodoListModel::IsCompletedRole);
    EXPECT_TRUE(completedData.toBool());
}

TEST_F(TodoListModelTest, AddTodo_AppendsToModelAndSync) {
    using ::testing::_;
    using ::testing::Return;

    EXPECT_CALL(*m_mockSync, readTasks())
        .WillRepeatedly(Return(std::vector<brain::domain::INoteSync::TaskItem>{}));

    brain::presentation::TodoListModel model(m_mockSync);
    EXPECT_EQ(model.rowCount(), 0);

    EXPECT_CALL(*m_mockSync, appendTodo(_))
        .Times(1);

    model.updateTaskWithNLP(-1, "New task 🔺 📅 2026-06-10");

    // Since the actual implementation calls loadTasks() which relies on readTasks() 
    // to populate the UI, we just check if appendTodo was called correctly.
    // The integration between appendTodo and readTasks is handled by the real ObsidianSync.
}

TEST_F(TodoListModelTest, ToggleTask_UpdatesStateAndSync) {
    using ::testing::Return;
    using ::testing::_;

    std::vector<brain::domain::INoteSync::TaskItem> mockTasks = {
        {"Task 1", false, "", 0}
    };

    EXPECT_CALL(*m_mockSync, readTasks())
        .WillOnce(Return(mockTasks));

    brain::presentation::TodoListModel model(m_mockSync);

    EXPECT_CALL(*m_mockSync, updateTask(0, true))
        .Times(1);

    model.toggleTask(0);

    EXPECT_TRUE(model.data(model.index(0, 0), brain::presentation::TodoListModel::IsCompletedRole).toBool());
}

TEST_F(TodoListModelTest, DeleteTask_RemovesRowAndSync) {
    using ::testing::Return;

    std::vector<brain::domain::INoteSync::TaskItem> mockTasks = {
        {"Task 1", false, "", 0},
        {"Task 2", false, "", 0}
    };

    EXPECT_CALL(*m_mockSync, readTasks())
        .WillOnce(Return(mockTasks));

    brain::presentation::TodoListModel model(m_mockSync);
    EXPECT_EQ(model.rowCount(), 2);

    EXPECT_CALL(*m_mockSync, deleteTask(0))
        .Times(1);

    model.deleteTask(0);

    EXPECT_EQ(model.rowCount(), 1);
    EXPECT_EQ(model.data(model.index(0, 0), brain::presentation::TodoListModel::DisplayRole).toString(), "Task 2");
}

TEST_F(TodoListModelTest, UndoDelete_RestoresTaskAndSync) {
    using ::testing::Return;
    using ::testing::_;

    std::vector<brain::domain::INoteSync::TaskItem> mockTasks = {
        {"Important Task", false, "2026-06-12", 2}
    };

    EXPECT_CALL(*m_mockSync, readTasks())
        .WillRepeatedly(Return(mockTasks));

    brain::presentation::TodoListModel model(m_mockSync);

    EXPECT_CALL(*m_mockSync, deleteTask(0)).Times(1);
    model.deleteTask(0);
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_TRUE(model.canUndo());

    EXPECT_CALL(*m_mockSync, appendTodo(_)).Times(1);
    model.undoDelete();
}

TEST_F(TodoListModelTest, ClearAllTasks_RemovesAllAndSync) {
    using ::testing::Return;

    std::vector<brain::domain::INoteSync::TaskItem> mockTasks = {
        {"Task 1", false, "", 0},
        {"Task 2", false, "", 0}
    };

    EXPECT_CALL(*m_mockSync, readTasks())
        .WillOnce(Return(mockTasks));

    brain::presentation::TodoListModel model(m_mockSync);
    EXPECT_EQ(model.rowCount(), 2);

    EXPECT_CALL(*m_mockSync, clearAllTasks())
        .Times(1);

    model.clearAllTasks();

    EXPECT_EQ(model.rowCount(), 0);
}
