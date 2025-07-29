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
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::StartButtonClicked() {
    ui->stackedWidget->setCurrentIndex(2); // 切换到第三页
}

void MainWindow::onSettingExitClicked() {
    ui->stackedWidget->setCurrentIndex(0); // 切换回第一页
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

    // 额外输出s
    // QWidget* centralWidget = this->centralWidget();
    // QStackedWidget* stackedWidget = ui->stackedWidget;
    // QWidget* currentPage = stackedWidget->currentWidget();
    // qDebug() << "[resizeEvent] centralWidget size:" << centralWidget->size();
    // qDebug() << "[resizeEvent] stackedWidget size:" << stackedWidget->size();
    // qDebug() << "[resizeEvent] page size:" << currentPage->size();

    ui->verticalLayout_page->setStretch(0, 5); // Top spacer
    // ui->verticalLayout_page->setStretch(2, 0); // Between2 spacer
    // ui->verticalLayout_page->setStretch(4, 0); // Bottom spacer
    ui->verticalLayout_page->setStretch(6, 5); // Bottom spacer
}

void MainWindow::onSettingButtonClicked() {
    // 切换到第二页
    ui->stackedWidget->setCurrentIndex(1);
}

}// namespace flychess_client
