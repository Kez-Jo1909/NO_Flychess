#include "../include/MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>

namespace flychess_client{
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 连接信号和槽函数
    connect(ui->StartButton, &QPushButton::clicked, this, &MainWindow::StartButtonClicked);
    connect(ui->ExitButton, &QPushButton::clicked, this, &MainWindow::onExitButtonClicked);
    connect(ui->SettingButton, &QPushButton::clicked, this, &MainWindow::onSettingButtonClicked);
    connect(ui->SettingsExitButton, &QPushButton::clicked, this, &MainWindow::onSettingExitClicked);
    connect(ui->StartExitButton, &QPushButton::clicked, this, &MainWindow::onSettingExitClicked);
    connect(ui->SettingReButton, &QPushButton::clicked, this, &MainWindow::onSettingReButtonClicked);
    connect(ui->SettingSaveButton, &QPushButton::clicked, this, &MainWindow::onSettingSaveButtonClicked);
    connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonClicked);
    connect(ui->CreateGameButton, &QPushButton::clicked, this, &MainWindow::onCreateGameButtonClicked);

    // 绑定菜单栏-关于
    connect(ui->actionAbout, &QAction::triggered,
            this, &MainWindow::showAboutDialog);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::showAboutDialog() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("关于");

    // 设置富文本并允许点击链接
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setTextInteractionFlags(Qt::TextBrowserInteraction);

    QString text = R"(
        <h3>关于项目</h3>
        <p>这是一个基于 Qt 的飞行棋游戏客户端。</p>
        作者: KezJo<br>
        版本: 0.1.0<br>
        <a href='https://github.com/Kez-Jo1909/NO_Flychess'>访问 GitHub 项目主页</a>
    )";

    msgBox.setText(text);

    // 捕获链接点击
    QLabel* label = msgBox.findChild<QLabel*>("qt_msgbox_label");
    if (label) {
        QObject::connect(label, &QLabel::linkActivated,
                         [](const QString &link){
            QDesktopServices::openUrl(QUrl(link));
        });
    }

    msgBox.exec();
}

void MainWindow::StartButtonClicked() {
    ui->stackedWidget->setCurrentIndex(2); // 切换到第三页
}

void MainWindow::onSettingExitClicked() {
    ui->stackedWidget->setCurrentIndex(0); // 切换回第一页
}

void MainWindow::onCreateGameButtonClicked() {
    // 切换到创建游戏页面
    ui->stackedWidget->setCurrentIndex(3);
    ui->RoomListWidget->addItem("system: 创建游戏成功,进入房间");
}

void MainWindow::onPreparePageExitButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "返回确认",
        "确定要退出房间吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        ui->stackedWidget->setCurrentIndex(2);   
        ui->RoomListWidget->clear();
    }
}

void MainWindow::onSettingReButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "重置确认",
        "确定要重置设定吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        
    }
}

void MainWindow::onSettingSaveButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "保存确认",
        "确定要保存设定吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        
    }
}

void MainWindow::onExitButtonClicked() {
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

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event); // 保留父类处理

    ui->verticalLayout_page->setStretch(0, 3); // Top spacer
    // ui->verticalLayout_page->setStretch(2, 0); // Between2 spacer
    // ui->verticalLayout_page->setStretch(4, 0); // Bottom spacer
    ui->verticalLayout_page->setStretch(6, 1); // Bottom spacer

    ui->horizontalLayout_StartButton->setStretch(0, 19);
    ui->horizontalLayout_StartButton->setStretch(2, 1);

    ui->horizontalLayout_SettingsButton->setStretch(0, 19);
    ui->horizontalLayout_SettingsButton->setStretch(2, 1);

    ui->horizontalLayout_ExitButton->setStretch(0, 19);
    ui->horizontalLayout_ExitButton->setStretch(2, 1);
}

void MainWindow::onSettingButtonClicked() {
    // 切换到第二页
    ui->stackedWidget->setCurrentIndex(1);
}

}// namespace flychess_client
