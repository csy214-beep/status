#include "MainWindow.h"
#include "ManageDialog.h"
#include <QScreen>
#include <QGuiApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QAction>
#include <QApplication>
#include <QSettings>
#include <QDesktopServices>

MainWindow::MainWindow(StatusManager *manager, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
      , m_statusManager(manager)
      , m_dragging(false)
      , m_expanded(false) {
    setupUI();
    setupTrayIcon();

    // 连接状态变化信号
    connect(m_statusManager, &StatusManager::statusesChanged,
            this, &MainWindow::onStatusesChanged);

    // 设置定时器，每30秒刷新一次
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDisplay);
    m_refreshTimer->start(1000);

    // 初始显示
    refreshDisplay();

    // 加载保存的位置
    loadPosition();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // 设置窗口属性
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(5);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // 创建图标布局容器
    QWidget *iconContainer = new QWidget;
    m_iconLayout = new QGridLayout(iconContainer);
    m_iconLayout->setSpacing(ICON_SPACING);
    m_iconLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(iconContainer);

    // 创建展开按钮
    m_expandButton = new QPushButton("▼ 展开");
    m_expandButton->setFixedHeight(24);
    m_expandButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(255, 255, 255, 200);"
        "   border: 2px solid #e0e0e0;"
        "   border-radius: 4px;"
        "   padding: 2px 10px;"
        "   color: #666666;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 255, 255, 240);"
        "   border: 2px solid #4CAF50;"
        "}"
    );
    connect(m_expandButton, &QPushButton::clicked, this, &MainWindow::toggleExpanded);
    m_mainLayout->addWidget(m_expandButton);
    m_expandButton->hide(); // 初始隐藏

    // 创建空状态标签
    m_emptyLabel = new QLabel("当前未设置状态", this);
    m_emptyLabel->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(255, 255, 255, 200);"
        "   border: 2px solid #cccccc;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   color: #666666;"
        "   font-size: 14px;"
        "}"
    );
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->hide();
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("状态显示应用");

    // 创建托盘菜单
    m_trayMenu = new QMenu(this);
    QAction *manageAction = m_trayMenu->addAction("管理状态");
    QAction *refreshAction = m_trayMenu->addAction("刷新显示");
    QAction *folderAction = m_trayMenu->addAction("程序文件夹");
    m_trayMenu->addSeparator();
    QAction *quitAction = m_trayMenu->addAction("退出");

    connect(manageAction, &QAction::triggered, this, &MainWindow::openManageDialog);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshDisplay);
    connect(folderAction, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()));
    });
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::trayIconActivated);
}

void MainWindow::loadPosition() {
    QSettings settings("StatusApp", "StatusDisplayApp");
    QPoint pos = settings.value("windowPosition").toPoint();

    if (!pos.isNull()) {
        move(pos);
    } else {
        // 默认位置：屏幕右上角
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.right() - width() - 10;
        int y = screenGeometry.top() + 10;
        move(x, y);
    }
}

void MainWindow::savePosition() {
    QSettings settings("StatusApp", "StatusDisplayApp");
    settings.setValue("windowPosition", pos());
}

bool operator==(const QVector<StatusData> &lhs, const QVector<StatusData> &rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (int i = 0; i < lhs.size(); ++i) {
        if (!(lhs[i] == rhs[i])) return false;
    }
    return true;
}

void MainWindow::refreshDisplay() {
    QVector<StatusData> activeStatuses = m_statusManager->getActiveStatuses();
    if (m_statusesTemp == activeStatuses) {
        return;
    } else {
        clearIcons();
        m_statusesTemp = activeStatuses;
    }
    if (activeStatuses.isEmpty()) {
        // 显示空状态标签
        m_iconLayout->addWidget(m_emptyLabel, 0, 0);
        m_emptyLabel->show();
        m_expandButton->hide();
    } else {
        m_emptyLabel->hide();

        // 确定要显示的状态数量
        int displayCount = activeStatuses.size();
        bool showExpandButton = false;

        if (!m_expanded && activeStatuses.size() > DEFAULT_SHOW_COUNT) {
            displayCount = DEFAULT_SHOW_COUNT;
            showExpandButton = true;
        }

        // 显示图标
        int row = 0, col = 0;
        for (int i = 0; i < displayCount; ++i) {
            const StatusData &status = activeStatuses[i];
            createIconLabel(status);

            QLabel *label = m_iconLabels.last();
            m_iconLayout->addWidget(label, row, col);

            col++;
            if (col >= MAX_ICONS_PER_ROW) {
                col = 0;
                row++;
            }
        }

        // 显示或隐藏展开按钮
        if (showExpandButton) {
            m_expandButton->setText(QString("▼ 展开 (%1 更多)").arg(activeStatuses.size() - DEFAULT_SHOW_COUNT));
            m_expandButton->show();
        } else if (m_expanded && activeStatuses.size() > DEFAULT_SHOW_COUNT) {
            m_expandButton->setText("▲ 收起");
            m_expandButton->show();
        } else {
            m_expandButton->hide();
        }
    }

    // 调整窗口大小
    adjustSize();
}

void MainWindow::createIconLabel(const StatusData &status) {
    QLabel *label = new QLabel(this);

    // 设置图标文本（emoji）
    label->setText(status.icon);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(ICON_SIZE, ICON_SIZE);

    // 设置样式
    label->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(255, 255, 255, 180);"
        "   border: 2px solid #e0e0e0;"
        "   border-radius: 8px;"
        "   font-size: 32px;"
        "}"
        "QLabel:hover {"
        "   background-color: rgba(255, 255, 255, 220);"
        "   border: 2px solid #4CAF50;"
        "}"
    );

    // 设置Tooltip
    QString tooltip = QString("<div style='padding: 5px;'>"
                "<p style='font-size: 16px; margin: 0;'><b>%1 %2</b></p>"
                "<p style='margin: 5px 0 0 0; color: gray;'>%3</p>"
                "</div>")
            .arg(status.icon)
            .arg(status.description)
            .arg(status.getShortTimeRange());
    label->setToolTip(tooltip);

    m_iconLabels.append(label);
}

void MainWindow::clearIcons() {
    for (QLabel *label: m_iconLabels) {
        m_iconLayout->removeWidget(label);
        label->deleteLater();
    }
    m_iconLabels.clear();

    // 确保空标签从布局中移除
    if (m_emptyLabel->parent()) {
        m_iconLayout->removeWidget(m_emptyLabel);
    }
}

void MainWindow::onStatusesChanged() {
    refreshDisplay();
}

void MainWindow::openManageDialog() {
    ManageDialog dialog(m_statusManager, this);
    dialog.exec();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        openManageDialog();
    }
}

void MainWindow::toggleExpanded() {
    m_expanded = !m_expanded;
    refreshDisplay();
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        // 右键点击显示菜单
        m_trayMenu->exec(event->globalPosition().toPoint());
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        // 保存新位置
        savePosition();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 保存位置
    savePosition();
    QWidget::closeEvent(event);
}
