#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <vector>
#include <memory>

namespace brain::infrastructure {
    class ObsidianSync;
}

namespace brain::presentation {

class TodoListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        IsCompletedRole = Qt::UserRole + 1,
        DueDateRole = Qt::UserRole + 2,
        PriorityRole = Qt::UserRole + 3
    };

    struct TodoItem {
        QString text;
        bool isCompleted;
        QString dueDate;
        int priority; // 3 = High, 2 = Normal, 1 = Low
    };

    explicit TodoListModel(std::shared_ptr<brain::infrastructure::ObsidianSync> sync, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void toggleTask(int index);
    Q_INVOKABLE void updateTaskText(int index, const QString& newText);
    Q_INVOKABLE void deleteTask(int index);

    Q_INVOKABLE void loadTasks();

private:
    std::shared_ptr<brain::infrastructure::ObsidianSync> m_sync;
    std::vector<TodoItem> m_tasks;
};

} // namespace brain::presentation
