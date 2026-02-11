#ifndef MANAGEDIALOG_H
#define MANAGEDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include "StatusManager.h"

/**
 * @brief 状态管理对话框，用于添加和查看状态
 */
class ManageDialog : public QDialog {
    Q_OBJECT

public:
    explicit ManageDialog(StatusManager *manager, QWidget *parent = nullptr);

private slots:
    void addStatus();
    void deleteStatus();
    void refreshAllTable();
    void refreshCurrentTable();
    void refreshExpiredTable();
    void onTabChanged(int index);

private:
    void setupUI();
    void createAddTab();
    void createViewTabs();
    void populateTable(QTableWidget *table, const QVector<StatusData> &statuses);

    StatusManager *m_statusManager;

    // 添加状态选项卡
    QLineEdit *m_iconEdit;
    QLineEdit *m_descEdit;
    QDateTimeEdit *m_startDateTimeEdit;  // 改为日期时间选择器
    QSpinBox *m_durationSpin;
    QPushButton *m_addButton;

    // 查看状态选项卡
    QTabWidget *m_mainTabs;
    QTableWidget *m_allTable;
    QTableWidget *m_currentTable;
    QTableWidget *m_expiredTable;
};

#endif // MANAGEDIALOG_H