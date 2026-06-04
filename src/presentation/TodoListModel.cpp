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
    if (!index.isValid() || index.row() < 0 || static_cast<std::size_t>(index.row()) >= m_tasks.size()) return {};

    const auto& item = m_tasks[static_cast<std::size_t>(index.row())];
    if (role == DisplayRole) {
        return item.text;
    } else if (role == IsCompletedRole) {
        return item.isCompleted;
    }
    return {};
}

QHash<int, QByteArray> TodoListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[DisplayRole] = "display";
    roles[IsCompletedRole] = "isCompleted";
    return roles;
}

void TodoListModel::toggleTask(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= m_tasks.size()) return;
    
    auto& task = m_tasks[index];
    task.isCompleted = !task.isCompleted;
    
    m_sync->updateTask(index, task.isCompleted);
    
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {IsCompletedRole});
}

void TodoListModel::updateTaskText(int index, const QString& newText) {
    if (index < 0 || index >= static_cast<int>(m_tasks.size())) return;
    if (newText.trimmed().isEmpty()) return;

    auto& task = m_tasks[index];
    if (task.text == newText) return;

    task.text = newText;
    m_sync->updateTaskText(index, newText.toStdString());

    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {DisplayRole});
}

void TodoListModel::deleteTask(int index) {
    if (index < 0 || index >= static_cast<int>(m_tasks.size())) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.erase(m_tasks.begin() + index);
    endRemoveRows();

    m_sync->deleteTask(index);
}

void TodoListModel::loadTasks() {
    beginResetModel();
    m_tasks.clear();
    if (m_sync) {
        auto parsedTasks = m_sync->readTasks();
        for (const auto& pt : parsedTasks) {
            m_tasks.push_back({QString::fromStdString(pt.text), pt.isCompleted});
        }
    }
    endResetModel();
}

} // namespace brain::presentation
