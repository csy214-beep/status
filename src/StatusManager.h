#ifndef STATUSMANAGER_H
#define STATUSMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include "StatusData.h"

/**
 * @brief 状态管理器，负责状态数据的读写和管理
 */
class StatusManager : public QObject {
    Q_OBJECT

public:
    explicit StatusManager(QObject *parent = nullptr);

    /**
     * @brief 从JSON文件加载状态数据
     */
    bool loadFromFile(const QString &filePath);

    /**
     * @brief 保存状态数据到JSON文件
     */
    bool saveToFile(const QString &filePath);

    /**
     * @brief 获取所有状态
     */
    QVector<StatusData> getAllStatuses() const { return m_statuses; }

    /**
     * @brief 获取当前有效的状态
     */
    QVector<StatusData> getActiveStatuses() const;

    /**
     * @brief 获取已过期的状态
     */
    QVector<StatusData> getExpiredStatuses() const;

    /**
     * @brief 添加新状态
     */
    void addStatus(const StatusData &status);

    /**
     * @brief 删除指定索引的状态
     */
    void removeStatus(int index);

    /**
     * @brief 清空所有状态
     */
    void clearAll();

    signals:
        /**
         * @brief 状态数据变化信号
         */
        void statusesChanged();

private:
    QVector<StatusData> m_statuses;
    QString m_filePath;
};

#endif // STATUSMANAGER_H