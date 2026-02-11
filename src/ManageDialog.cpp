#include "ManageDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QScreen>

ManageDialog::  ManageDialog(StatusManager *manager, QWidget *parent)
    : QDialog(parent)
    , m_statusManager(manager)
{
    setupUI();
    
    // 连接状态变化信号
    connect(m_statusManager, &StatusManager::statusesChanged,
            this, &ManageDialog::refreshAllTable);
    connect(m_statusManager, &StatusManager::statusesChanged,
            this, &ManageDialog::refreshCurrentTable);
    connect(m_statusManager, &StatusManager::statusesChanged,
            this, &ManageDialog::refreshExpiredTable);
    
    // 初始刷新
    refreshAllTable();
    refreshCurrentTable();
    refreshExpiredTable();


}

void ManageDialog::setupUI() {
    setWindowTitle("状态管理");
    setMinimumSize(800, 600);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建主选项卡
    m_mainTabs = new QTabWidget(this);
    
    createAddTab();
    createViewTabs();
    
    mainLayout->addWidget(m_mainTabs);
    
    connect(m_mainTabs, &QTabWidget::currentChanged,
            this, &ManageDialog::onTabChanged);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    int x = (screenRect.width()  - width()) /2;
    int y = (screenRect.height() - height()) /2;
    move(x, y);
}

void ManageDialog::createAddTab() {
    QWidget *addWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(addWidget);
    
    // 创建输入表单
    QGroupBox *inputGroup = new QGroupBox("添加新状态");
    QFormLayout *formLayout = new QFormLayout(inputGroup);
    
    m_iconEdit = new QLineEdit;
    m_iconEdit->setPlaceholderText("输入emoji或图标，例如：😀 📚 ⚡");
    m_iconEdit->setMaxLength(2);
    formLayout->addRow("图标:", m_iconEdit);
    
    m_descEdit = new QLineEdit;
    m_descEdit->setPlaceholderText("例如：工作时间、休息时间");
    formLayout->addRow("状态描述:", m_descEdit);
    
    // 使用日期时间选择器
    m_startDateTimeEdit = new QDateTimeEdit;
    m_startDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_startDateTimeEdit->setCalendarPopup(true);  // 启用日历弹出
    m_startDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_startDateTimeEdit->setMinimumWidth(200);
    formLayout->addRow("起始时间:", m_startDateTimeEdit);

    m_durationSpin = new QSpinBox;
    m_durationSpin->setRange(1, 168);  // 最多一周
    m_durationSpin->setValue(8);
    m_durationSpin->setSuffix(" 小时");
    formLayout->addRow("持续时长:", m_durationSpin);

    layout->addWidget(inputGroup);

    // 添加按钮
    m_addButton = new QPushButton("添加状态");
    m_addButton->setStyleSheet("QPushButton { padding: 10px; font-size: 14px; }");
    connect(m_addButton, &QPushButton::clicked, this, &ManageDialog::addStatus);
    layout->addWidget(m_addButton);

    // 说明文本
    QLabel *helpLabel = new QLabel(
        "<b>使用说明：</b><br>"
        "• 图标：可以使用emoji表情符号<br>"
        "• 常用emoji：😀 😊 😴 📚 💻 ⚡ 🏃 🍔 ☕ 🌙 ⏰ 📝<br>"
        "• Windows快捷键：Win + . (句号) 打开emoji面板<br>"
        "• 起始时间：点击可选择年月日和具体时间<br>"
        "• 持续时长：状态持续的小时数（1-168小时）<br>"
        "• 示例：2026-02-11 09:00 开始，持续8小时 = 09:00-17:00"
    );
    helpLabel->setWordWrap(true);
    helpLabel->setStyleSheet("QLabel { padding: 15px; background-color: #f0f0f0; border-radius: 5px; }");
    layout->addWidget(helpLabel);

    layout->addStretch();

    m_mainTabs->addTab(addWidget, "添加状态");
}

void ManageDialog::createViewTabs() {
    // 所有状态选项卡
    QWidget *allWidget = new QWidget;
    QVBoxLayout *allLayout = new QVBoxLayout(allWidget);

    m_allTable = new QTableWidget;
    m_allTable->setColumnCount(5);
    m_allTable->setHorizontalHeaderLabels({"图标", "描述", "起始时间", "持续时长", "时间范围"});
    m_allTable->horizontalHeader()->setStretchLastSection(true);
    m_allTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_allTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    allLayout->addWidget(m_allTable);

    QPushButton *deleteAllBtn = new QPushButton("删除选中");
    connect(deleteAllBtn, &QPushButton::clicked, this, &ManageDialog::deleteStatus);
    allLayout->addWidget(deleteAllBtn);

    m_mainTabs->addTab(allWidget, "所有状态");

    // 当前状态选项卡
    QWidget *currentWidget = new QWidget;
    QVBoxLayout *currentLayout = new QVBoxLayout(currentWidget);

    m_currentTable = new QTableWidget;
    m_currentTable->setColumnCount(5);
    m_currentTable->setHorizontalHeaderLabels({"图标", "描述", "起始时间", "持续时长", "时间范围"});
    m_currentTable->horizontalHeader()->setStretchLastSection(true);
    m_currentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_currentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    currentLayout->addWidget(m_currentTable);

    m_mainTabs->addTab(currentWidget, "当前状态");

    // 过期状态选项卡
    QWidget *expiredWidget = new QWidget;
    QVBoxLayout *expiredLayout = new QVBoxLayout(expiredWidget);

    m_expiredTable = new QTableWidget;
    m_expiredTable->setColumnCount(5);
    m_expiredTable->setHorizontalHeaderLabels({"图标", "描述", "起始时间", "持续时长", "时间范围"});
    m_expiredTable->horizontalHeader()->setStretchLastSection(true);
    m_expiredTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_expiredTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    expiredLayout->addWidget(m_expiredTable);

    m_mainTabs->addTab(expiredWidget, "过期状态");
}

void ManageDialog::addStatus() {
    if (m_iconEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入图标！");
        return;
    }

    if (m_descEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入状态描述！");
        return;
    }

    StatusData status;
    status.icon = m_iconEdit->text().trimmed();
    status.description = m_descEdit->text().trimmed();
    status.startDateTime = m_startDateTimeEdit->dateTime();
    status.durationHours = m_durationSpin->value();

    m_statusManager->addStatus(status);

    // 清空输入
    m_iconEdit->clear();
    m_descEdit->clear();
    m_startDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_durationSpin->setValue(8);

    QMessageBox::information(this, "成功", "状态已添加！");
}

void ManageDialog::deleteStatus() {
    int currentRow = m_allTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "未选中", "请先选择要删除的状态！");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除", "确定要删除选中的状态吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_statusManager->removeStatus(currentRow);
    }
}

void ManageDialog::populateTable(QTableWidget *table, const QVector<StatusData> &statuses) {
    table->setRowCount(statuses.size());

    for (int i = 0; i < statuses.size(); ++i) {
        const StatusData &status = statuses[i];

        table->setItem(i, 0, new QTableWidgetItem(status.icon));
        table->setItem(i, 1, new QTableWidgetItem(status.description));
        table->setItem(i, 2, new QTableWidgetItem(status.startDateTime.toString("yyyy-MM-dd hh:mm")));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(status.durationHours) + " 小时"));
        table->setItem(i, 4, new QTableWidgetItem(status.getTimeRange()));
        
        // 设置图标列的字体大小
        QFont iconFont = table->item(i, 0)->font();
        iconFont.setPointSize(20);
        table->item(i, 0)->setFont(iconFont);
        table->item(i, 0)->setTextAlignment(Qt::AlignCenter);
    }
    
    table->resizeColumnsToContents();
}

void ManageDialog::refreshAllTable() {
    populateTable(m_allTable, m_statusManager->getAllStatuses());
}

void ManageDialog::refreshCurrentTable() {
    populateTable(m_currentTable, m_statusManager->getActiveStatuses());
}

void ManageDialog::refreshExpiredTable() {
    populateTable(m_expiredTable, m_statusManager->getExpiredStatuses());
}

void ManageDialog::onTabChanged(int index) {
    // 切换到查看选项卡时刷新数据
    if (index == 1) {
        refreshAllTable();
    } else if (index == 2) {
        refreshCurrentTable();
    } else if (index == 3) {
        refreshExpiredTable();
    }
}