#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPushButton>
#include "StatusManager.h"

/**
 * @brief 主显示窗口，显示当前有效的状态图标
 */
class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(StatusManager *manager, QWidget *parent = nullptr);

    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

private slots:
    void refreshDisplay();

    void onStatusesChanged();

    void openManageDialog();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void toggleExpanded();

private:
    void setupUI();

    void setupTrayIcon();

    void loadPosition();

    void savePosition();

    void createIconLabel(const StatusData &status);

    void clearIcons();

    StatusManager *m_statusManager;
    QVBoxLayout *m_mainLayout;
    QGridLayout *m_iconLayout;
    QTimer *m_refreshTimer;
    QVector<QLabel *> m_iconLabels;
    QLabel *m_emptyLabel;
    QPushButton *m_expandButton;

    // 缓存状态数据
    QVector<StatusData> m_statusesTemp = {StatusData()};


    // 系统托盘
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;

    // 窗口拖动
    QPoint m_dragPosition;
    bool m_dragging;

    // 展开状态
    bool m_expanded;

    static const int MAX_ICONS_PER_ROW = 8;
    static const int ICON_SIZE = 48;
    static const int ICON_SPACING = 10;
    static const int DEFAULT_SHOW_COUNT = 3; // 默认显示数量
};

#endif // MAINWINDOW_H
