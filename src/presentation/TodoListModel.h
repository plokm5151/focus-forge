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
        IsCompletedRole = Qt::UserRole + 1
    };

    struct TodoItem {
        QString text;
        bool isCompleted;
    };

    explicit TodoListModel(std::shared_ptr<brain::infrastructure::ObsidianSync> sync, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void toggleTask(int index);

    void loadTasks();

private:
    std::shared_ptr<brain::infrastructure::ObsidianSync> m_sync;
    std::vector<TodoItem> m_tasks;
};

} // namespace brain::presentation
