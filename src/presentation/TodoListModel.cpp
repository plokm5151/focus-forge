#include "TodoListModel.h"
#include "../infrastructure/ObsidianSync.h"
#include <QDate>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

namespace brain::presentation {

TodoListModel::TodoListModel(std::shared_ptr<brain::infrastructure::ObsidianSync> sync, QObject* parent)
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
    beginResetModel();
    m_tasks.clear();
    
    auto syncTasks = m_sync->readTasks();
    for (const auto& st : syncTasks) {
        m_tasks.push_back(TodoItem{
            QString::fromStdString(st.text),
            st.isCompleted,
            QString::fromStdString(st.dueDate),
            st.priority
        });
    }

    // Auto-sort: High priority tasks float to top
    sortByPriority();

    endResetModel();
}

void TodoListModel::toggleTask(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;
    
    auto& task = m_tasks[static_cast<std::size_t>(index)];
    task.isCompleted = !task.isCompleted;
    
    m_sync->updateTask(index, task.isCompleted);
    
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {IsCompletedRole});
}

void TodoListModel::updateTaskText(int index, const QString& newText) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;
    
    auto& task = m_tasks[static_cast<std::size_t>(index)];
    task.text = newText;
    
    brain::domain::INoteSync::TaskItem syncTask;
    syncTask.text = newText.toStdString();
    syncTask.isCompleted = task.isCompleted;
    syncTask.dueDate = task.dueDate.toStdString();
    syncTask.priority = task.priority;
    
    m_sync->updateTaskText(index, syncTask);
    
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DisplayRole});
}

void TodoListModel::deleteTask(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;

    // Save for undo
    m_lastDeletedItem = m_tasks[static_cast<std::size_t>(index)];
    m_lastDeletedIndex = index;
    m_canUndo = true;

    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.erase(m_tasks.begin() + index);
    endRemoveRows();

    m_sync->deleteTask(index);

    // Adjust pinned index
    if (m_pinnedIndex == index) {
        m_pinnedIndex = -1;
        emit pinnedIndexChanged();
    } else if (m_pinnedIndex > index) {
        m_pinnedIndex--;
        emit pinnedIndexChanged();
    }
}

void TodoListModel::undoDelete() {
    if (!m_canUndo) return;

    // Re-insert the deleted task
    int insertAt = std::min(m_lastDeletedIndex, static_cast<int>(m_tasks.size()));
    beginInsertRows(QModelIndex(), insertAt, insertAt);
    m_tasks.insert(m_tasks.begin() + insertAt, m_lastDeletedItem);
    endInsertRows();

    // Also re-insert into Obsidian file by reloading
    brain::domain::INoteSync::TaskItem syncTask;
    syncTask.text = m_lastDeletedItem.text.toStdString();
    syncTask.isCompleted = m_lastDeletedItem.isCompleted;
    syncTask.dueDate = m_lastDeletedItem.dueDate.toStdString();
    syncTask.priority = m_lastDeletedItem.priority;
    m_sync->appendTodo(syncTask);

    m_canUndo = false;
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
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;

    auto& task = m_tasks[static_cast<std::size_t>(index)];
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

    if (text.contains(todayRe)) {
        task.dueDate = QDate::currentDate().toString("yyyy-MM-dd");
        text.remove(todayRe);
    } else if (text.contains(tmrwRe)) {
        task.dueDate = QDate::currentDate().addDays(1).toString("yyyy-MM-dd");
        text.remove(tmrwRe);
    }

    task.text = text.trimmed();

    // Persist
    brain::domain::INoteSync::TaskItem syncTask;
    syncTask.text = task.text.toStdString();
    syncTask.isCompleted = task.isCompleted;
    syncTask.dueDate = task.dueDate.toStdString();
    syncTask.priority = task.priority;
    m_sync->updateTaskText(index, syncTask);

    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DisplayRole, PriorityRole, DueDateRole});

    // Re-sort after priority change
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

} // namespace brain::presentation
