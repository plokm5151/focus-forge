#include "TodoListModel.h"
#include "../infrastructure/ObsidianSync.h"
#include <QDebug>

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

    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.erase(m_tasks.begin() + index);
    endRemoveRows();

    m_sync->deleteTask(index);
}

} // namespace brain::presentation
