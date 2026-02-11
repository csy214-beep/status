#include "StatusManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

StatusManager::StatusManager(QObject *parent)
    : QObject(parent)
{
}

bool StatusManager::loadFromFile(const QString &filePath) {
    m_filePath = filePath;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "JSON格式错误";
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray statusArray = root["statuses"].toArray();
    
    m_statuses.clear();
    for (const QJsonValue &value : statusArray) {
        QJsonObject obj = value.toObject();
        StatusData status;
        status.icon = obj["icon"].toString();
        status.description = obj["description"].toString();
        status.durationHours = obj["durationHours"].toInt();

        // 支持新格式（日期时间字符串）
        if (obj.contains("startDateTime")) {
            status.startDateTime = QDateTime::fromString(
                obj["startDateTime"].toString(), Qt::ISODate);
        }
        // 兼容旧格式（仅小时）
        else if (obj.contains("startHour")) {
            int startHour = obj["startHour"].toInt();
            QDateTime now = QDateTime::currentDateTime();
            status.startDateTime = QDateTime(now.date(), QTime(startHour, 0));
        }

        m_statuses.append(status);
    }

    emit statusesChanged();
    return true;
}

bool StatusManager::saveToFile(const QString &filePath) {
    QJsonArray statusArray;

    for (const StatusData &status : m_statuses) {
        QJsonObject obj;
        obj["icon"] = status.icon;
        obj["description"] = status.description;
        obj["startDateTime"] = status.startDateTime.toString(Qt::ISODate);
        obj["durationHours"] = status.durationHours;
        statusArray.append(obj);
    }
    
    QJsonObject root;
    root["statuses"] = statusArray;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入文件:" << filePath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

QVector<StatusData> StatusManager::getActiveStatuses() const {
    QVector<StatusData> active;
    for (const StatusData &status : m_statuses) {
        if (status.isActive()) {
            active.append(status);
        }
    }
    return active;
}

QVector<StatusData> StatusManager::getExpiredStatuses() const {
    QVector<StatusData> expired;
    for (const StatusData &status : m_statuses) {
        if (status.isExpired()) {
            expired.append(status);
        }
    }
    return expired;
}

void StatusManager::addStatus(const StatusData &status) {
    m_statuses.append(status);
    if (!m_filePath.isEmpty()) {
        saveToFile(m_filePath);
    }
    emit statusesChanged();
}

void StatusManager::removeStatus(int index) {
    if (index >= 0 && index < m_statuses.size()) {
        m_statuses.remove(index);
        if (!m_filePath.isEmpty()) {
            saveToFile(m_filePath);
        }
        emit statusesChanged();
    }
}

void StatusManager::clearAll() {
    m_statuses.clear();
    if (!m_filePath.isEmpty()) {
        saveToFile(m_filePath);
    }
    emit statusesChanged();
}