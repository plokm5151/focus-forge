#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <vector>
#include <memory>
#include <QSortFilterProxyModel>

#include "domain/INoteSync.h"

namespace brain::presentation {

class TodoListModel : public QAbstractListModel {
    Q_OBJECT

    /** @brief Index of the pinned/focused task (-1 = none). */
    Q_PROPERTY(int pinnedIndex READ pinnedIndex WRITE setPinnedIndex NOTIFY pinnedIndexChanged)
    Q_PROPERTY(QString pinnedTaskText READ pinnedTaskText NOTIFY pinnedIndexChanged)

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
        int originalIndex; // Index in the Obsidian file
    };

    explicit TodoListModel(std::shared_ptr<brain::domain::INoteSync> sync, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void toggleTask(int index);
    Q_INVOKABLE void updateTaskText(int index, const QString& newText);
    Q_INVOKABLE void deleteTask(int index);
    Q_INVOKABLE void loadTasks();

    /** @brief Undo the last deletion (within 5 seconds). */
    Q_INVOKABLE void undoDelete();

    /** @brief Check if an undo is available. */
    Q_INVOKABLE bool canUndo() const;

    /** @brief Pin a task as the current focus objective. */
    Q_INVOKABLE void pinTask(int index);

    /** @brief Unpin the current focus objective. */
    Q_INVOKABLE void unpinTask();

    /** @brief Update task text with inline NLP parsing (priority/date tags). */
    Q_INVOKABLE void updateTaskWithNLP(int index, const QString& rawText);

    [[nodiscard]] int pinnedIndex() const;
    void setPinnedIndex(int index);
    [[nodiscard]] QString pinnedTaskText() const;

signals:
    void pinnedIndexChanged();
    void pinnedTaskTextChanged();

private:
    void sortByPriority();

    std::shared_ptr<brain::domain::INoteSync> m_sync;
    std::vector<TodoItem> m_tasks;
    int m_pinnedIndex{-1};

    // Undo support
    TodoItem m_lastDeletedItem;
    int m_lastDeletedIndex{-1};
    bool m_canUndo{false};
};

class ActiveTaskFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit ActiveTaskFilterModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
        setDynamicSortFilter(true);
    }
    Q_INVOKABLE int mapRowToSource(int proxyRow) const {
        return mapToSource(index(proxyRow, 0)).row();
    }
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        return !sourceModel()->data(index, TodoListModel::IsCompletedRole).toBool();
    }
};

class HistoryTaskFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit HistoryTaskFilterModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
        setDynamicSortFilter(true);
    }
    Q_INVOKABLE int mapRowToSource(int proxyRow) const {
        return mapToSource(index(proxyRow, 0)).row();
    }
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        return sourceModel()->data(index, TodoListModel::IsCompletedRole).toBool();
    }
};

} // namespace brain::presentation
