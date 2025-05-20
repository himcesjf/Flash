#include "logmodel.h"

LogModel::LogModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    
    return m_entries.size();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return QVariant();
    }
    
    const LogEntry &entry = m_entries.at(index.row());
    
    switch (role) {
        case TimestampRole:
            return entry.timestamp;
        case TimestampStrRole:
            return entry.timestamp.toString("HH:mm:ss.zzz");
        case LevelRole:
            return entry.level;
        case LevelStrRole:
            return levelToString(entry.level);
        case MessageRole:
            return entry.message;
        case ColorRole:
            return levelToColor(entry.level);
        case Qt::DisplayRole:
            return QString("[%1] %2: %3")
                   .arg(entry.timestamp.toString("HH:mm:ss"))
                   .arg(levelToString(entry.level))
                   .arg(entry.message);
        default:
            return QVariant();
    }
}

void LogModel::addMessage(int level, const QString &message)
{
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.message = message;
    
    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.append(entry);
    endInsertRows();
    
    // Keep a maximum of 1000 log entries
    if (m_entries.size() > 1000) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_entries.removeFirst();
        endRemoveRows();
    }
}

void LogModel::clear()
{
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimestampRole] = "timestamp";
    roles[TimestampStrRole] = "timestampStr";
    roles[LevelRole] = "level";
    roles[LevelStrRole] = "levelStr";
    roles[MessageRole] = "message";
    roles[ColorRole] = "color";
    return roles;
}

QString LogModel::levelToString(int level) const
{
    switch (level) {
        case 0: return "DEBUG";
        case 1: return "INFO";
        case 2: return "WARN";
        case 3: return "ERROR";
        default: return "UNKNOWN";
    }
}

QString LogModel::levelToColor(int level) const
{
    switch (level) {
        case 0: return "#808080";  // Gray
        case 1: return "#000000";  // Black
        case 2: return "#FF8800";  // Orange
        case 3: return "#FF0000";  // Red
        default: return "#000000"; // Black
    }
} 