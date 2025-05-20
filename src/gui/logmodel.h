#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>

/**
 * @brief The LogEntry struct represents a single log message
 */
struct LogEntry {
    QDateTime timestamp;
    int level;
    QString message;
};

/**
 * @brief The LogModel class provides a model for log messages
 */
class LogModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum LogRoles {
        TimestampRole = Qt::UserRole + 1,
        TimestampStrRole,
        LevelRole,
        LevelStrRole,
        MessageRole,
        ColorRole
    };

    explicit LogModel(QObject *parent = nullptr);
    
    /**
     * @brief Get number of rows in the model
     * @param parent Parent index
     * @return Row count
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    
    /**
     * @brief Get data for a role at an index
     * @param index Model index
     * @param role Data role
     * @return Data value
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    /**
     * @brief Add a log message
     * @param level Log level (0=debug, 1=info, 2=warning, 3=error)
     * @param message Log message text
     */
    void addMessage(int level, const QString &message);
    
    /**
     * @brief Clear all log messages
     */
    void clear();

protected:
    /**
     * @brief Get role names for QML
     * @return Map of role names
     */
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<LogEntry> m_entries;
    
    /**
     * @brief Get string representation of log level
     * @param level Log level
     * @return Level string
     */
    QString levelToString(int level) const;
    
    /**
     * @brief Get color for log level
     * @param level Log level
     * @return Color string
     */
    QString levelToColor(int level) const;
};

#endif // LOGMODEL_H 