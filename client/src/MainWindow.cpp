#include "../include/MainWindow.h"
#include "game.h"
#include "ui_MainWindow.h"
#include "utils.h"
#include <QDebug>
#include <qlist.h>
#include <qobject.h>
#include <vector>

namespace flychess_client{
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->RoomListTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->RoomTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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
    // connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonClicked);
    connect(ui->CreateGameButton, &QPushButton::clicked, this, &MainWindow::onCreateGameButtonClicked);
    // connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePageStartButtonClicked);
    connect(ui->UrlEdit, &QLineEdit::returnPressed, this, &MainWindow::UrlEditEnter);
    connect(ui->PlayerCountBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPlayerCountChanged);
    connect(ui->ChessCountBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onChessCountChanged);
    connect(ui->RollDiceButton, &QPushButton::clicked, this, &MainWindow::onRollDiceButton);

    connect(client_, &FlychessClient::connected,    this, &MainWindow::onConnected);
    connect(client_, &FlychessClient::disconnected, this, &MainWindow::onDisconnected);

    // 绑定菜单栏-关于
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    connect(client_, &FlychessClient::newPlayerJoined, this, &MainWindow::onNewPlayerJoined);
    connect(client_, &FlychessClient::registerResult, this, &MainWindow::onRegisterResult);
    connect(client_, &FlychessClient::playerListUpdated, this, &MainWindow::onPlayerListUpdated);
    connect(client_, &FlychessClient::allPieceInfo, this, &MainWindow::onAllPieceInfo);
    connect(client_, &FlychessClient::playerLeaveRoom, this, &MainWindow::onPlayerLeaveRoom);
    connect(client_, &FlychessClient::updatePlayerCount, this, &MainWindow::onPlayerCountUpdate);
    connect(client_, &FlychessClient::updateChessCount, this, &MainWindow::onChessCountUpdate);
    connect(client_, &FlychessClient::updatePlayerCountFailed, this, &MainWindow::onPlayerCountUpdateFailed);
    connect(client_, &FlychessClient::GameStart, this, &MainWindow::onGameStart);
    connect(client_, &FlychessClient::GameStartFailed, this, &MainWindow::onGameStartFailed);
    connect(client_, &FlychessClient::GameStartNotEnough, this, &MainWindow::onGameStartNotEnough);
    connect(client_, &FlychessClient::rollDiceResult, this, &MainWindow::onRollDiceResult);
    connect(this, &MainWindow::AllPieceInfo, ui->chessBoardWidget, &ChessBoardWidget::updatePieces);
    connect(client_, &FlychessClient::toRollDice, this, &MainWindow::onToRollDice);
    connect(client_, &FlychessClient::OtherToRollDice, this, &MainWindow::onOtherToRollDice);
    connect(ui->chessBoardWidget, &ChessBoardWidget::selectedChessPiece, this, &MainWindow::onSelectedChessPiece);
    connect(client_, &FlychessClient::toUseCard, this, &MainWindow::onToUseCard);
    connect(client_, &FlychessClient::NoAvailableChess, this, &MainWindow::onNoAvailableChess);
    connect(client_, &FlychessClient::SomeoneFinished, this, &MainWindow::onSomeoneFinished);
    connect(client_, &FlychessClient::AllPlayerFinished, this, &MainWindow::onAllPlayerFinished);

    // 初始化定时器
    connectTimer_ = new QTimer(this);
    connectTimer_->setSingleShot(true);  // 只触发一次
    connect(connectTimer_, &QTimer::timeout, this, &MainWindow::onConnectTimeout);

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onNewPlayerJoined(QString name, game_utils::Color color) {
    std::string color_str = game_utils::colorToString(color);
    QString message = QString("system: %1进入房间, 颜色: %2").arg(name, QString::fromStdString(color_str));
    ui->RoomListWidget->addItem(message);   
}

void MainWindow::onGameStart() {
    ui->stackedWidget->setCurrentIndex(4);
    ui->RollDiceButton->setEnabled(false);
    this->player_state_ = flychess_game::PlayerState::WAITING;
}

void MainWindow::onAllPlayerFinished(const QList<QVariantList>& rank_list) {
    std::vector<std::string> color_rank;
    for (const QVariantList& rank : rank_list) {
        int color = rank[0].toInt();
        color_rank.push_back(game_utils::colorIntToString(color));
    }

    // 拼接显示内容
    QString msg = "游戏结束，排名如下：\n";
    for (size_t i = 0; i < color_rank.size(); ++i) {
        msg += QString("第%1名: %2\n").arg(i + 1).arg(QString::fromStdString(color_rank[i]));
    }

    QMessageBox::information(this, "排名结果", msg);
    // TODO 解算页面

    // 回到房间
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::onSomeoneFinished(int color) {
    if (static_cast<int>(user_color_) == color) {
        // auto reply = QMessageBox::question(
        //     this,
        //     "留在房间确认",
        //     "已完成游戏\n还要留在房间吗?",
        //     QMessageBox::Yes | QMessageBox::No
        // );
        // if (reply == QMessageBox::Yes) {
        //     // nothing to do
        // } else {
        //     ui->stackedWidget->setCurrentIndex(3);
        // }
        QMessageBox::warning(this, "恭喜", "您已经完成游戏");
        // 很屎的修复
        this->client_->sendFinishUseCard(color);
    } else {
        // nothing to do?
    }
}

void MainWindow::onToUseCard() {
    this->player_state_ = flychess_game::PlayerState::CARDING;
    
    // TODO
    // 由于没有牌，先在这里直接跳过
    this->client_->sendFinishUseCard(static_cast<int>(user_color_));
    this->player_state_ = flychess_game::PlayerState::WAITING;
}

void MainWindow::onNoAvailableChess(int color) {
    if (static_cast<int>(user_color_) != color) {
        ui->DiceTextLabel->setText("玩家无可用棋子...");
    }
    else {
        ui->DiceTextLabel->setText("无可用棋子\n自动跳过选择...");
        this->player_state_ = flychess_game::PlayerState::WAITING;

        QTimer::singleShot(500, this, [this]() {
            this->client_->sendFinishTextWaiting();
        });
    }
}

void MainWindow::onGameStartFailed() {
    QMessageBox::warning(this, "游戏开始失败", "有玩家未准备好，无法开始游戏！");
}

void MainWindow::onGameStartNotEnough() {
    QMessageBox::warning(this, "游戏开始失败", "玩家人数不足，无法开始游戏！");
}

void MainWindow::onPlayerLeaveRoom(QString name, game_utils::Color color) {
    std::string color_str = game_utils::colorToString(color);
    QString message = QString("system: %1(%2) 离开房间").arg(name, QString::fromStdString(color_str));
    ui->RoomListWidget->addItem(message);  
}

void MainWindow::onPlayerCountUpdate(int num) {
    int index = 4 - num;
    if (index >= 0 && index < ui->PlayerCountBox->count()) {
        ui->PlayerCountBox->setCurrentIndex(index);
    }
}

void MainWindow::onRollDiceButton() {
    this->client_->sendRollDiceRequest();
    ui->RollDiceButton->setEnabled(false);
    ui->DiceTextLabel->setText("正在掷骰子,请稍等...");
}

void MainWindow::onRollDiceResult(int result, QString player_name, int player_color) {
    if (this->player_state_ == flychess_game::PlayerState::ROLLING) {
        ui->DiceTextLabel->setText("掷骰结果: " + QString::number(result) + "\n请选择棋子");
        this->player_state_ = flychess_game::PlayerState::SELECTING;
    } else{
        std::string color_str = game_utils::colorIntToString(player_color);
        QString message = QString("玩家 %1 (%2) \n 掷骰结果: %3")
                            .arg(player_name)
                            .arg(QString::fromStdString(color_str))
                            .arg(result);
        // ui->DiceTextLabel->setWordWrap(true); // 启用自动换行
        ui->DiceTextLabel->setText(message);
    }
}

void MainWindow::onToRollDice() {
    player_state_ = flychess_game::PlayerState::ROLLING;
    ui->DiceTextLabel->setText("请掷骰子...");
    ui->RollDiceButton->setEnabled(true);
}

void MainWindow::onOtherToRollDice(int color) {
    if (this->player_state_ == flychess_game::PlayerState::ROLLING) return;
    
    std::string color_str = game_utils::colorIntToString(color);
    ui->DiceTextLabel->setText("等待 " + QString::fromStdString(color_str) + " 玩家掷骰子...");
}

void MainWindow::onChessCountUpdate(int num) {
    int index = 4 - num;
    if (index >= 0 && index < ui->ChessCountBox->count()) {
        ui->ChessCountBox->setCurrentIndex(index);
    }
}

void MainWindow::onPlayerCountUpdateFailed(int min_num) {
    QMessageBox::warning(this,
                         "错误",
                         QString("当前房间已有 %1 人!").arg(min_num));
}

void MainWindow::onRegisterResult(game_utils::Color color) {
    this->user_color_ = color;
    std::string color_str = game_utils::colorToString(color);
    QString message = QString("system: 当前颜色: %1").arg(QString::fromStdString(color_str));
    ui->RoomListWidget->addItem(message);  
}

void MainWindow::onPlayerCountChanged(int index) {
    // 通过索引获取文本，例如 "3人"
    QString text = ui->PlayerCountBox->itemText(index);

    // 去掉最后的“人”，并转为整数
    int playerCount = text.left(text.length() - 1).toInt();

    // qDebug() << "当前选择人数:" << playerCount;
    client_->sendPlayerCount(playerCount);
}

void MainWindow::onChessCountChanged(int index) {
    QString text = ui->ChessCountBox->itemText(index);

    // 去掉最后的“子”，并转为整数
    int chess_count = text.left(text.length() - 1).toInt();

    client_->sendChessCount(chess_count);
}

void MainWindow::onSelectedChessPiece(int id, int color) {
    if (this->player_state_ != flychess_game::PlayerState::SELECTING) {
        std::cout << "当前非选择棋子状态" << std::endl;
        return;
    }

    if (color != static_cast<int>(user_color_)) {
        std::cout << "非当前玩家的棋子" << std::endl;
        return;
    }

    this->client_->sendChosenChessPiece(id, color);
    this->player_state_ = flychess_game::PlayerState::WAITING;
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

void MainWindow::onAllPieceInfo(const QList<QVariantList>& pieces) {
    if (pieces.isEmpty()) {
        qDebug() << "Received empty piece info.";
        return;
    }

    if (!chess_pieces_.empty()) {
        // 清空之前的棋子信息
        chess_pieces_.clear();
    }

    for (const QVariantList& piece : pieces) {
        int id = piece[0].toInt();
        int color = piece[1].toInt();
        int position = piece[2].toInt();
        int player_id = piece[3].toInt();
        chess_pieces_.push_back(flychess_game::ChessPieceInfo(id, static_cast<game_utils::Color>(color), position, player_id));
    }

    // 之后直接转发信号
    emit AllPieceInfo(pieces);
}

void MainWindow::onPlayerListUpdated(const QList<QVariantList>& players) {
    // 清空表格
    ui->RoomTableWidget->setRowCount(0);

    // 遍历玩家列表
    int row = 0;
    for (const QVariantList& player : players) {
        // 确保 player 里有三列：[name(QString), colorInt(int), is_ready(bool)]
        if (player.size() < 3) continue;

        QString name = player[0].toString();
        int colorInt = player[1].toInt();
        bool isReady = player[2].toBool();

        // 插入新行
        ui->RoomTableWidget->insertRow(row);

        // ID 列（这里我理解你想显示玩家名字）
        ui->RoomTableWidget->setItem(row, 0, new QTableWidgetItem(name));

        // 颜色列（转换成字符串）
        std::string colorStr = game_utils::colorIntToString(colorInt);
        QString colorQStr = QString::fromStdString(colorStr);
        ui->RoomTableWidget->setItem(row, 1, new QTableWidgetItem(colorQStr));

        // 准备列
        ui->RoomTableWidget->setItem(row, 2, new QTableWidgetItem(isReady ? "已准备" : "未准备"));

        row++;
    }
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

void MainWindow::onPreparePageStartButtonClicked() {
    // 开始游戏
    auto reply = QMessageBox::question(
        this,
        "开始游戏",
        "确定要开始游戏吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        server_->GameStart();
        // ui->stackedWidget->setCurrentIndex(4);
    }
}

void MainWindow::onPreparePagePrepareButtonClicked() {
    if(prepared){
        ui->PreparePageStartButton->setText("准备");
        this->client_->sendGetUnPrepared();
    } else {
        ui->PreparePageStartButton->setText("取消准备");
        this->client_->sendGetPrepared();
    }
    prepared = !prepared;
}

void MainWindow::onConnected() {
    // 停止超时计时
    connectTimer_->stop();  

    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }
    // QMessageBox::information(this, "提示", "连接服务器成功！");
    if (is_server) {
        ui->PlayerCountBox->setEnabled(true);
        ui->ChessCountBox->setEnabled(true);
        ui->ifCardCheckBox->setEnabled(true);
        ui->ifAiCheckBox->setEnabled(true);
        ui->PreparePageStartButton->setText("开始游戏");
        disconnect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonClicked);
        disconnect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePageStartButtonClicked);
        connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePageStartButtonClicked);
        connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonClicked);
        // ui->stackedWidget->setCurrentIndex(3);
    } else {
        ui->PlayerCountBox->setEnabled(false);
        ui->ChessCountBox->setEnabled(false);
        ui->ifCardCheckBox->setEnabled(false);
        ui->ifAiCheckBox->setEnabled(false);
        ui->PreparePageStartButton->setText("准备");
        disconnect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePagePrepareButtonClicked);
        disconnect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonUserClicked);
        connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePagePrepareButtonClicked);
        connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonUserClicked);
        // ui->stackedWidget->setCurrentIndex(3);
    }
    ui->stackedWidget->setCurrentIndex(3);
    // 准备发送用户信息
    client_->sendUserInfo("player");

    if(server_) {
        // 默认准备
        this->client_->sendGetPrepared();
    }
}

void MainWindow::onDisconnected() {
    connectTimer_->stop(); // 如果是断开也需要停掉定时器

    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }

    if (!is_server) {
        client_->close();
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

void MainWindow::onCreateGameButtonClicked() {
    is_server = true;

    // ui->stackedWidget->setCurrentIndex(3);

    server_ = std::make_unique<flychess_server::FlychessServer>(8080);

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
            // game_ = new flychess_game::FlychessGame();

            QMetaObject::invokeMethod(this, [this]() {
                ui->RoomListWidget->addItem("system: 创建游戏成功,服务器已建立");
                ui->RoomListWidget->addItem("system: 本地用户进入房间");
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void MainWindow::onPreparePageExitButtonUserClicked() {
    auto reply = QMessageBox::question(
        this,
        "返回确认",
        "确定要退出房间吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        client_->close();
        ui->stackedWidget->setCurrentIndex(2);   
        ui->RoomListWidget->clear();
    }
}


void MainWindow::onPreparePageExitButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "返回确认",
        "确定要退出并解散房间吗？",
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

    // ui->horizontalLayout_6->setStretch(0, 1);
    // ui->horizontalLayout_6->setStretch(1, 4);
    // ui->horizontalLayout_6->setStretch(2, 1);

    ui->verticalLayout_GamePage->setStretch(0, 5);
    ui->verticalLayout_GamePage->setStretch(1, 2);

    ui->horizontalLayout_9->setStretch(0,6);
    ui->horizontalLayout_9->setStretch(1,2);
    ui->horizontalLayout_9->setStretch(2,2);
    ui->horizontalLayout_9->setStretch(3,2);
    ui->horizontalLayout_9->setStretch(4,6);

    ui->verticalLayout_5->setStretch(0, 5); // Top spacer
    ui->verticalLayout_5->setStretch(1, 1); // Between spacer

    ui->verticalLayout_8->setStretch(0, 1); // Top spacer
    ui->verticalLayout_8->setStretch(1, 1); // Between spacer

}

void MainWindow::onSettingButtonClicked() {
    // 切换到第二页
    ui->stackedWidget->setCurrentIndex(1);
}

}// namespace flychess_client


ChessBoardWidget::ChessBoardWidget(QWidget *parent)
    : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize ChessBoardWidget::minimumSizeHint() const {
    return QSize(300, 300);
}

// void ChessBoardWidget::resizeEvent(QResizeEvent *event) {
//     int size = std::min(width(), height());
//     resize(size, size); // 保持正方形
//     QWidget::resizeEvent(event);
// }

void ChessBoardWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算最大正方形区域
    boardSizePx = std::min(width(), height()) / 34 * 34;
    offsetX = (width()  - boardSizePx) / 2;
    offsetY = (height() - boardSizePx) / 2;

    grid_size = boardSizePx / 17; // 每个格子的大小
    radius = boardSizePx / 51;

    // 绘制整个控件背景
    painter.fillRect(rect(), Qt::white);

    // 绘制棋盘区域的浅色背景
    painter.fillRect(QRect(offsetX, offsetY, boardSizePx, boardSizePx), QColor(255, 255, 255));

    // 绘制正方形边框
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(offsetX, offsetY, boardSizePx, boardSizePx);

    int grid_num = flychess_map::GetGridCount();
    for (int i = 0; i < grid_num; i++) {
        auto grid = flychess_map::GetGridInfo(i);

        // std::cout << "grid_id" << grid->id << std::endl;
        if (grid->id == -2) {
            continue;
        }

        int type = grid->type;
        std::vector<int> color_vector = game_utils::colorintToRGB(grid->color);
        int p_x = grid->position_x / 40 * grid_size + offsetX;
        int p_y = grid->position_y / 40 * grid_size + offsetY;
        // std::cout<<"读取width:" << grid->width <<std::endl;

        if(type == 0 || type == 2) {
            // 矩形
            int width = grid->width / 40 * grid_size;
            // std::cout<<"type0 size:" << width << std::endl;
            int height = grid->height / 40 * grid_size;
            QRect rect(p_x, p_y, width, height);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
            painter.drawRect(rect);


            // 画圆
            // int center_x = p_x + width / 2;
            // int center_y = p_y + height / 2;
            std::pair<int,int> center_position = getGridCenter(p_x, p_y, width, height, type);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 绘制圆形
        }
        else if(type == 3) {
            p_x = offsetX + boardSizePx / 2;
            p_y = offsetY + boardSizePx / 2;
            int c_x = p_x;
            int c_y = p_y;
            int height = grid->height;
            int width = grid->width / 40.0f * grid_size;// 这里一定是40.0f * grid_size,float不能省
            // std::cout<<"type3 size:" << width << std::endl;
            painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
            QPolygonF triangle;
            if (height == 0) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x - width, p_y - width)
                        << QPointF(p_x + width, p_y - width);
                // c_y -= width / 1.5;
            }
            else if (height == 1) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x + width, p_y - width)
                        << QPointF(p_x + width, p_y + width);
                // c_x += width / 1.5;
            }
            else if (height == 2) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x - width, p_y + width)
                        << QPointF(p_x + width, p_y + width);
                // c_y += width / 1.5;
            }
            else if (height == 3) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x - width, p_y + width)
                        << QPointF(p_x - width, p_y - width);
                // c_x -= width / 1.5;
            }
            painter.drawPolygon(triangle); // 绘制三角形

            // 画圆
            std::pair<int,int> center_position = getGridCenter(p_x, p_y, width, height, type);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 绘制圆形
        }
        else {
            int height = grid->height;
            int width = grid->width / 40.0f * grid_size;
            painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
            QPolygonF triangle;
            int c_x = p_x, c_y = p_y;
            if (height == 0) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x + width, p_y)
                        << QPointF(p_x, p_y + width);
                // c_x += width / 3.0;
                // c_y += width / 3.0;
            }
            else if (height == 1) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x - width, p_y)
                        << QPointF(p_x, p_y + width);
                // c_x -= width / 3.0;
                // c_y += width / 3.0;
            }
            else if (height == 2) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x, p_y - width)
                        << QPointF(p_x - width, p_y);
                // c_x -= width / 3.0;
                // c_y -= width / 3.0;            
            }
            else if (height == 3) {
                triangle << QPointF(p_x, p_y)
                        << QPointF(p_x, p_y - width)
                        << QPointF(p_x + width, p_y);
                // c_x += width / 3.0;
                // c_y -= width / 3.0;            
            }

            painter.drawPolygon(triangle); // 绘制三角形

            // 画圆
            std::pair<int,int> center_position = getGridCenter(p_x, p_y, width, height, type);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); 
        }
    }

    // 绘制棋子
    // std::cout<< "size:" << chess_pieces_.size() <<std::endl;
    for(int i = 0; i < chess_pieces_.size(); i++) {
        int c_position = chess_pieces_[i].position;
        auto grid_info = &flychess_map::getGameMap().searchGridInfo(c_position, static_cast<int>(chess_pieces_[i].color), chess_pieces_[i].id);
        if (grid_info == nullptr) {
            std::cerr << "Error: Grid info not found for chess piece at position " << c_position << std::endl;
            continue;
        }

        int p_x = grid_info->position_x / 40 * grid_size + offsetX;
        int p_y = grid_info->position_y / 40 * grid_size + offsetY;
        int width = grid_info->width / 40 * grid_size;
        int type = grid_info->type;
        std::vector<int> color_vector = game_utils::colorintToRGB(static_cast<int> (chess_pieces_[i].color));
        std::pair<int,int> center_position;
        
        // std::cout << "grid_id" << grid_info->id << std::endl;
        if (grid_info->id == -2) {
            continue;
        }

        if (type == 0 || type == 2) {
            int height = grid_info->height / 40 * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
            // painter.setPen(QPen(Qt::black, 1));
            // // std::vector<int> color_vector = game_utils::colorintToRGB(static_cast<int> (chess_pieces_[i].color));
            // painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
            // painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 绘制圆形
        }
        else if (type == 3){
            p_x = offsetX + boardSizePx / 2;
            p_y = offsetY + boardSizePx / 2;
            int height = grid_info->height;
            width = grid_info->width / 40.0f * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
            // painter.setPen(QPen(Qt::black, 1));
            // // std::vector<int> color_vector = game_utils::colorintToRGB(static_cast<int> (chess_pieces_[i].color));
            // painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
            // painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 绘制圆形
        }
        else {
            int height = grid_info->height;
            int width = grid_info->width / 40.0f * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
        }
        painter.setPen(QPen(Qt::black, 1));
        // std::vector<int> color_vector = game_utils::colorintToRGB(static_cast<int> (chess_pieces_[i].color));
        painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
        painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 绘制圆形
    }
}

void ChessBoardWidget::updatePieces(const QList<QVariantList> &pieces) {
    // 清空之前的棋子信息
    chess_pieces_.clear();

    // 遍历接收到的棋子信息
    for (const QVariantList &piece : pieces) {
        int id = piece[0].toInt();
        int color = piece[1].toInt();
        int position = piece[2].toInt();
        int player_id = piece[3].toInt();
        chess_pieces_.emplace_back(id, static_cast<game_utils::Color>(color), position, player_id);
    }

    // 触发重绘
    update();
}

std::pair<int,int> ChessBoardWidget::getGridCenter(int px, int py, int width, int height, int type){
    int c_x = px, c_y = py;
    if(type == 0 || type == 2){
        c_x += width / 2;
        c_y += height / 2;
    }
    else if (type == 3){
        if (height == 0) {
            c_y -= width / 1.5;
        }
        else if (height == 1) {
            c_x += width / 1.5;
        }
        else if (height == 2) {
            c_y += width / 1.5;
        }
        else if (height == 3) {
            c_x -= width / 1.5;
        }
    }
    else{
        if (height == 0) {
            c_x += width / 3.0;
            c_y += width / 3.0;
        }
        else if (height == 1) {
            c_x -= width / 3.0;
            c_y += width / 3.0;
        }
        else if (height == 2) {
            c_x -= width / 3.0;
            c_y -= width / 3.0;            
        }
        else if (height == 3) {
            c_x += width / 3.0;
            c_y -= width / 3.0;            
        }   
    }
    return std::pair<int,int> (c_x, c_y);
}

void ChessBoardWidget::mousePressEvent(QMouseEvent *event) {
    QPoint pos = event->pos(); // 点击位置

    // qDebug() << "点击坐标:" << pos;
    // 计算棋盘区域的大小
    // int actual_x = pos.x() - offsetX;
    // int actual_y = pos.y() - offsetY;
    // qDebug() << "实际坐标:" << actual_x << actual_y;

    // 查找这是哪个格子
    for (auto chess_piece : chess_pieces_) {
        int c_position = chess_piece.position;
        auto grid_info = &flychess_map::getGameMap().searchGridInfo(c_position, static_cast<int>(chess_piece.color), chess_piece.id);
        auto type = grid_info->type;    
        int p_x = grid_info->position_x / 40 * grid_size + offsetX;
        int p_y = grid_info->position_y / 40 * grid_size + offsetY;
        int width = grid_info->width / 40 * grid_size;

        std::pair<int,int> center_position;
        if (type == 0 || type == 2) {
            int height = grid_info->height / 40 * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
        }
        else if (type == 3) {
            p_x = offsetX + boardSizePx / 2;
            p_y = offsetY + boardSizePx / 2;
            int height = grid_info->height;
            width = grid_info->width / 40.0f * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
        }
        else {
            int height = grid_info->height;
            int width = grid_info->width / 40.0f * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
        }

        double dx = center_position.first - pos.x();
        double dy = center_position.second - pos.y();
        if (dx * dx + dy * dy <= radius * radius) {
            int id = chess_piece.id;
            game_utils::Color picked_color = chess_piece.color;
            qDebug() << "点击了棋子ID:" << id << "颜色:" << game_utils::colorToString(picked_color).c_str();
            emit selectedChessPiece(id, static_cast<int>(picked_color));
            return;
        }
    }
}