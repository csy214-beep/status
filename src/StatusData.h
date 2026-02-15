#ifndef STATUSDATA_H
#define STATUSDATA_H

#include <QString>
#include <QDateTime>

/**
 * @brief 状态数据结构
 */
struct StatusData {
    QString icon; // 图标标识（emoji或图标名称）
    QString description; // 状态描述
    QDateTime startDateTime; // 起始时间（年月日时分）
    int durationHours; // 持续时长（小时）

    // 兼容旧格式的构造函数
    StatusData() : durationHours(1) {
        startDateTime = QDateTime::currentDateTime();
    }

    bool operator==(const StatusData &other) const {
        return icon == other.icon &&
               description == other.description &&
               startDateTime == other.startDateTime &&
               durationHours == other.durationHours;
    }

    /**
     * @brief 检查状态是否在当前时间有效
     */
    bool isActive() const {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime endDateTime = startDateTime.addSecs(durationHours * 3600);

        return now >= startDateTime && now < endDateTime;
    }

    /**
     * @brief 检查状态是否已过期
     */
    bool isExpired() const {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime endDateTime = startDateTime.addSecs(durationHours * 3600);

        return now >= endDateTime;
    }

    /**
     * @brief 获取时间范围字符串
     */
    QString getTimeRange() const {
        QDateTime endDateTime = startDateTime.addSecs(durationHours * 3600);
        return QString("%1 - %2")
                .arg(startDateTime.toString("yyyy-MM-dd hh:mm"))
                .arg(endDateTime.toString("yyyy-MM-dd hh:mm"));
    }

    /**
     * @brief 获取简短的时间范围（仅时间）
     */
    QString getShortTimeRange() const {
        QDateTime endDateTime = startDateTime.addSecs(durationHours * 3600);

        // 如果是同一天，只显示时间
        if (startDateTime.date() == endDateTime.date()) {
            return QString("%1 - %2")
                    .arg(startDateTime.toString("hh:mm"))
                    .arg(endDateTime.toString("hh:mm"));
        } else {
            // 跨天显示日期
            return QString("%1 - %2")
                    .arg(startDateTime.toString("MM-dd hh:mm"))
                    .arg(endDateTime.toString("MM-dd hh:mm"));
        }
    }
};

#endif // STATUSDATA_H
