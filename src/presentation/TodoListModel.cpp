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
    if (!index.isValid() || index.row() >= m_tasks.size()) return {};

    const auto& item = m_tasks[index.row()];
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
    if (index < 0 || index >= m_tasks.size()) return;
    
    m_tasks[index].isCompleted = !m_tasks[index].isCompleted;
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {IsCompletedRole});

    // Save back to Obsidian
    if (m_sync) {
        m_sync->updateTask(index, m_tasks[index].isCompleted);
    }
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
