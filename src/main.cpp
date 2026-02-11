#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "MainWindow.h"
#include "StatusManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 设置应用信息
    app.setApplicationName("状态显示应用");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("StatusApp");

    // 创建状态管理器
    StatusManager manager;

    // 确定JSON文件路径
    QString jsonPath = QDir::currentPath() + "/status.json";

    // 如果文件不存在，创建一个默认文件
    if (!QFile::exists(jsonPath)) {
        QFile file(jsonPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("{\n  \"statuses\": []\n}");
            file.close();
        }
    }

    // 加载状态数据
    if (!manager.loadFromFile(jsonPath)) {
        QMessageBox::critical(nullptr, "错误",
            "无法加载状态数据文件：" + jsonPath);
        return 1;
    }

    // 创建并显示主窗口
    MainWindow window(&manager);
    window.show();

    return app.exec();
}