#include "../include/MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>

namespace flychess_client{
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 创建客户端实例
    client_ = new flychess_client::FlychessClient(this);

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
    connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePageStartButtonClicked);
    connect(ui->UrlEdit, &QLineEdit::returnPressed, this, &MainWindow::UrlEditEnter);

    connect(client_, &FlychessClient::connected,    this, &MainWindow::onConnected);
    connect(client_, &FlychessClient::disconnected, this, &MainWindow::onDisconnected);

    // 绑定菜单栏-关于
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // 初始化定时器
    connectTimer_ = new QTimer(this);
    connectTimer_->setSingleShot(true);  // 只触发一次
    connect(connectTimer_, &QTimer::timeout, this, &MainWindow::onConnectTimeout);

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

void MainWindow::UrlEditEnter() {
    QString url = ui->UrlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入服务器URL!");
        return;
    }
    // qDebug() << "按回车输入的URL是:" << url;
    auto reply = QMessageBox::question(
        this,
        "加入房间确认",
        "确定要加入房间吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        is_server = false;
        // 弹出“正在连接”提示框（非阻塞）
        connectingBox_ = new QMessageBox(QMessageBox::Information,
                                        "连接中",
                                        "正在连接服务器，请稍候…",
                                        QMessageBox::NoButton,
                                        this);
        connectingBox_->setModal(false);
        connectingBox_->show();

        // 启动超时计时器（5秒）
        connectTimer_->start(5000);

        client_->connectToServer("ws://" + url.toStdString());
    }
}

void MainWindow::onConnected() {
    // 停止超时计时
    connectTimer_->stop();  

    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }
    QMessageBox::information(this, "提示", "连接服务器成功！");
    if (is_server) {
        ui->stackedWidget->setCurrentIndex(3);
    } else {
        ui->stackedWidget->setCurrentIndex(5);
    }
}

void MainWindow::onDisconnected() {
    connectTimer_->stop(); // 如果是断开也需要停掉定时器

    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }

    if (!is_server) {
        QMessageBox::warning(this, "提示", "连接失败或已断开！\n 即将返回上一个界面");
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::onConnectTimeout() {
    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }
    QMessageBox::warning(this, "提示", "连接超时，请检查服务器是否可用！");
    // 这里可以选择断开 WebSocket
    client_->disconnectFromServer();
}

void MainWindow::StartButtonClicked() {
    ui->stackedWidget->setCurrentIndex(2); // 切换到第三页
}

void MainWindow::onSettingExitClicked() {
    ui->stackedWidget->setCurrentIndex(0); // 切换回第一页
}

void MainWindow::onPreparePageStartButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "开始确认",
        "确定要开始游戏吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        std::cout << "start" << std::endl;
    }
}

void MainWindow::onCreateGameButtonClicked() {
    is_server = true;

    // ui->stackedWidget->setCurrentIndex(3);

    server_ = std::make_unique<flychess_server::FlycehssServer>(8080);

    std::thread([this] {
        bool server_ret = server_->start();

        if (!server_ret) {
            QMetaObject::invokeMethod(this, [this]() {
                QMessageBox::critical(this, "错误", "服务器启动失败！");
                ui->stackedWidget->setCurrentIndex(2);
                ui->RoomListWidget->clear();
            }, Qt::QueuedConnection);
        } else {
            // 这里发起本地客户端连接
            client_->connectToServer("ws://127.0.0.1:8080");
            QMetaObject::invokeMethod(this, [this]() {
                ui->RoomListWidget->addItem("system: 创建游戏成功,服务器已建立");
                ui->RoomListWidget->addItem("system: 本地用户进入房间");
            }, Qt::QueuedConnection);
        }
    }).detach();
}




void MainWindow::onPreparePageExitButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "返回确认",
        "确定要退出房间吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (server_) {
            client_->close();
            server_->stop();

            // 延迟执行reset，确保后台线程有时间退出
            QTimer::singleShot(100, this, [this]() {
                server_.reset();
                std::cout << "server stopped." << std::endl;
                ui->RoomListWidget->addItem("system: 服务器已关闭");
            });
        }

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

    ui->verticalLayout_3->setStretch(0, 3); // Top spacer
    ui->verticalLayout_3->setStretch(1, 1); // Between spacer

    ui->horizontalLayout_3->setStretch(0,2);
    ui->horizontalLayout_3->setStretch(1,1);

    ui->horizontalLayout_4->setStretch(0,2);
    ui->horizontalLayout_4->setStretch(1,1);
}

void MainWindow::onSettingButtonClicked() {
    // 切换到第二页
    ui->stackedWidget->setCurrentIndex(1);
}

}// namespace flychess_client
