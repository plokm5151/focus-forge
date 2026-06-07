#include "TodoListModel.h"
#include "../infrastructure/ObsidianSync.h"
#include <QDate>
#include <QDateTime>
#include <QTimeZone>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

namespace brain::presentation {

TodoListModel::TodoListModel(std::shared_ptr<brain::domain::INoteSync> sync, QObject* parent)
    : QAbstractListModel(parent), m_sync(std::move(sync))
{
    loadTasks();
}

int TodoListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_tasks.size());
}

QVariant TodoListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || static_cast<std::size_t>(index.row()) >= m_tasks.size()) {
        return QVariant{};
    }

    const auto& item = m_tasks[static_cast<std::size_t>(index.row())];
    switch (role) {
        case DisplayRole: return item.text;
        case IsCompletedRole: return item.isCompleted;
        case DueDateRole: return item.dueDate;
        case PriorityRole: return item.priority;
        default: return QVariant{};
    }
}

QHash<int, QByteArray> TodoListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[DisplayRole] = "display";
    roles[IsCompletedRole] = "isCompleted";
    roles[DueDateRole] = "dueDate";
    roles[PriorityRole] = "priority";
    return roles;
}

void TodoListModel::loadTasks() {
    int oldPinnedOriginalIndex = -1;
    if (m_pinnedIndex >= 0 && static_cast<std::size_t>(m_pinnedIndex) < m_tasks.size()) {
        oldPinnedOriginalIndex = m_tasks[static_cast<std::size_t>(m_pinnedIndex)].originalIndex;
    }

    beginResetModel();
    m_tasks.clear();
    m_pinnedIndex = -1;
    
    auto syncTasks = m_sync->readTasks();
    int originalIdx = 0;
    for (const auto& st : syncTasks) {
        m_tasks.push_back(TodoItem{
            QString::fromStdString(st.text),
            st.isCompleted,
            QString::fromStdString(st.dueDate),
            st.priority,
            originalIdx++
        });
    }

    // Auto-sort: High priority tasks float to top
    sortByPriority();

    // Restore pinned index
    if (oldPinnedOriginalIndex != -1) {
        for (std::size_t i = 0; i < m_tasks.size(); ++i) {
            if (m_tasks[i].originalIndex == oldPinnedOriginalIndex) {
                m_pinnedIndex = static_cast<int>(i);
                break;
            }
        }
    }

    endResetModel();

    emit pinnedIndexChanged();
    emit pinnedTaskTextChanged();
}

void TodoListModel::toggleTask(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;
    
    auto& task = m_tasks[static_cast<std::size_t>(index)];
    task.isCompleted = !task.isCompleted;
    
    m_sync->updateTask(task.originalIndex, task.isCompleted);
    
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {IsCompletedRole});
}

void TodoListModel::updateTaskText(int index, const QString& newText) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;
    
    auto& task = m_tasks[static_cast<std::size_t>(index)];
    if (task.text == newText) return; // Optimize: zero allocation if unchanged
    
    task.text = newText;
    
    brain::domain::INoteSync::TaskItem syncTask;
    syncTask.text = newText.toStdString();
    syncTask.isCompleted = task.isCompleted;
    syncTask.dueDate = task.dueDate.toStdString();
    syncTask.priority = task.priority;
    
    m_sync->updateTaskText(task.originalIndex, syncTask);
    
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DisplayRole});
}

void TodoListModel::deleteTask(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;

    // Save for undo
    m_lastDeletedItem = m_tasks[static_cast<std::size_t>(index)];
    m_lastDeletedIndex = index;
    m_canUndo = true;

    int deletedOriginalIndex = m_lastDeletedItem.originalIndex;

    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.erase(m_tasks.begin() + index);
    endRemoveRows();

    m_sync->deleteTask(deletedOriginalIndex);

    // Shift originalIndex for remaining tasks to match the new file structure
    for (auto& t : m_tasks) {
        if (t.originalIndex > deletedOriginalIndex) {
            t.originalIndex--;
        }
    }

    // Adjust pinned index
    if (m_pinnedIndex == index) {
        m_pinnedIndex = -1;
        emit pinnedIndexChanged();
        emit pinnedTaskTextChanged();
    } else if (m_pinnedIndex > index) {
        m_pinnedIndex--;
        emit pinnedIndexChanged();
    }
}

void TodoListModel::undoDelete() {
    if (!m_canUndo) return;

    // Re-insert into Obsidian file
    brain::domain::INoteSync::TaskItem syncTask;
    syncTask.text = m_lastDeletedItem.text.toStdString();
    syncTask.isCompleted = m_lastDeletedItem.isCompleted;
    syncTask.dueDate = m_lastDeletedItem.dueDate.toStdString();
    syncTask.priority = m_lastDeletedItem.priority;
    m_sync->appendTodo(syncTask);

    m_canUndo = false;
    
    // To ensure indices sync perfectly, we reload from disk
    loadTasks();
}

bool TodoListModel::canUndo() const {
    return m_canUndo;
}

void TodoListModel::pinTask(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;
    m_pinnedIndex = index;
    emit pinnedIndexChanged();
}

void TodoListModel::unpinTask() {
    m_pinnedIndex = -1;
    emit pinnedIndexChanged();
}

int TodoListModel::pinnedIndex() const {
    return m_pinnedIndex;
}

void TodoListModel::setPinnedIndex(int index) {
    if (m_pinnedIndex == index) return;
    m_pinnedIndex = index;
    emit pinnedIndexChanged();
}

QString TodoListModel::pinnedTaskText() const {
    if (m_pinnedIndex < 0 || static_cast<std::size_t>(m_pinnedIndex) >= m_tasks.size()) {
        return {};
    }
    return m_tasks[static_cast<std::size_t>(m_pinnedIndex)].text;
}

void TodoListModel::updateTaskWithNLP(int index, const QString& rawText) {
    if (index < -1 || (index >= 0 && static_cast<std::size_t>(index) >= m_tasks.size())) return;

    TodoItem task;
    if (index >= 0) {
        task = m_tasks[static_cast<std::size_t>(index)];
    } else {
        task.isCompleted = false;
        task.priority = 0;
        // The original index will be determined by loadTasks
    }
    
    QString text = rawText.trimmed();

    // Parse priority tags
    static QRegularExpression highRe(R"(\s*!(high|h)\b)", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression medRe(R"(\s*!(med|m)\b)", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression lowRe(R"(\s*!(low|l)\b)", QRegularExpression::CaseInsensitiveOption);

    if (text.contains(highRe)) {
        task.priority = 3;
        text.remove(highRe);
    } else if (text.contains(medRe)) {
        task.priority = 2;
        text.remove(medRe);
    } else if (text.contains(lowRe)) {
        task.priority = 1;
        text.remove(lowRe);
    }

    // Parse date tags
    static QRegularExpression todayRe(R"(\s*!(today|t)\b)", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression tmrwRe(R"(\s*!(tomorrow|tmrw)\b)", QRegularExpression::CaseInsensitiveOption);

    QDateTime twTime = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Taipei"));

    if (text.contains(todayRe)) {
        task.dueDate = twTime.date().toString("yyyy-MM-dd");
        text.remove(todayRe);
    } else if (text.contains(tmrwRe)) {
        task.dueDate = twTime.date().addDays(1).toString("yyyy-MM-dd");
        text.remove(tmrwRe);
    }

    task.text = text.trimmed();

    // Persist
    brain::domain::INoteSync::TaskItem syncTask;
    syncTask.text = task.text.toStdString();
    syncTask.isCompleted = task.isCompleted;
    syncTask.dueDate = task.dueDate.toStdString();
    syncTask.priority = task.priority;
    
    if (index >= 0) {
        m_sync->updateTaskText(task.originalIndex, syncTask);
        m_tasks[static_cast<std::size_t>(index)] = task;
        emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DisplayRole, PriorityRole, DueDateRole});
    } else {
        m_sync->appendTodo(syncTask);
    }

    // Re-sort and reload to sync indices
    loadTasks();
}

void TodoListModel::sortByPriority() {
    // Stable sort: High (3) first, then Med (2), Low (1), None (0)
    // Completed tasks sink to the bottom
    std::stable_sort(m_tasks.begin(), m_tasks.end(), [](const TodoItem& a, const TodoItem& b) {
        if (a.isCompleted != b.isCompleted) return !a.isCompleted; // uncompleted first
        return a.priority > b.priority; // higher priority first
    });
}
void TodoListModel::clearAllTasks() {
    if (!m_sync) return;

    beginResetModel();
    m_tasks.clear();
    unpinTask();
    m_sync->clearAllTasks();
    endResetModel();
}

} // namespace brain::presentation
