#include "../include/MainWindow.h"
#include "ui_MainWindow.h"

namespace flychess_client{
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 连接信号和槽函数
    connect(ui->StartButton, &QPushButton::clicked, this, &MainWindow::StartButtonClicked);

    connect(ui->ExitButton, &QPushButton::clicked, this, &MainWindow::on_ExitButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::StartButtonClicked() {
    
}

void MainWindow::on_ExitButton_clicked() {
    auto reply = QMessageBox::question(
        this,
        "退出确认",
        "确定要退出吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void MainWindow::repositionStartMenu() {
    // 获取中央控件的尺寸（不是整个窗口，而是内容区域）
    int w = ui->MainMenuWidget->width();
    int h = ui->MainMenuWidget->height();

    int btnW = ui->StartButton->width();
    int btnH = ui->StartButton->height();

    // 居中
    int x = w * 0.5 - btnW * 0.5;
    int y = h * 0.5 - btnH * 0.5;

    ui->StartButton->move(x, y);
    ui->SettingsButton->move(x, y + btnH + 10); // 设置按钮在开始按钮下方，间隔10像素
    ui->ExitButton->move(x, y + 2 * (btnH + 10)); // 退出按钮在设置按钮下方，间隔10像素

    // SakanaLabel位置
    int sakana_w = ui->SakanaLabel->width();
    int sakana_h = ui->SakanaLabel->height();

    // 区域右下角
    int sakana_x = w - sakana_w - 20; // 距离右边20像素
    int sakana_y = h - sakana_h - 20; // 距离底部20像素

    ui->SakanaLabel->move(sakana_x, sakana_y);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event); // 保留父类处理
    this->repositionStartMenu();
}

}// namespace flychess_client
