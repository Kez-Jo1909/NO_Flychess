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

    // 
    client_ = new flychess_client::FlychessClient(this);

    // 
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

    // -
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

    // 
    connectTimer_ = new QTimer(this);
    connectTimer_->setSingleShot(true);  // 
    connect(connectTimer_, &QTimer::timeout, this, &MainWindow::onConnectTimeout);

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onNewPlayerJoined(QString name, game_utils::Color color) {
    std::string color_str = game_utils::colorToString(color);
    QString message = QString("system: %1, : %2").arg(name, QString::fromStdString(color_str));
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

    // 
    QString msg = "\n";
    for (size_t i = 0; i < color_rank.size(); ++i) {
        msg += QString("%1: %2\n").arg(i + 1).arg(QString::fromStdString(color_rank[i]));
    }

    QMessageBox::information(this, "", msg);
    // TODO 

    // 
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::onSomeoneFinished(int color) {
    if (static_cast<int>(user_color_) == color) {
        // auto reply = QMessageBox::question(
        //     this,
        //     "",
        //     "\n?",
        //     QMessageBox::Yes | QMessageBox::No
        // );
        // if (reply == QMessageBox::Yes) {
        //     // nothing to do
        // } else {
        //     ui->stackedWidget->setCurrentIndex(3);
        // }
        QMessageBox::warning(this, "", "");
        // 
        this->client_->sendFinishUseCard(color);
    } else {
        // nothing to do?
    }
}

void MainWindow::onToUseCard() {
    this->player_state_ = flychess_game::PlayerState::CARDING;
    
    // TODO
    // 
    this->client_->sendFinishUseCard(static_cast<int>(user_color_));
    this->player_state_ = flychess_game::PlayerState::WAITING;
}

void MainWindow::onNoAvailableChess(int color) {
    if (static_cast<int>(user_color_) != color) {
        ui->DiceTextLabel->setText("...");
    }
    else {
        ui->DiceTextLabel->setText("\n...");
        this->player_state_ = flychess_game::PlayerState::WAITING;

        QTimer::singleShot(500, this, [this]() {
            this->client_->sendFinishTextWaiting();
        });
    }
}

void MainWindow::onGameStartFailed() {
    QMessageBox::warning(this, "", "");
}

void MainWindow::onGameStartNotEnough() {
    QMessageBox::warning(this, "", "");
}

void MainWindow::onPlayerLeaveRoom(QString name, game_utils::Color color) {
    std::string color_str = game_utils::colorToString(color);
    QString message = QString("system: %1(%2) ").arg(name, QString::fromStdString(color_str));
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
    ui->DiceTextLabel->setText(",...");
}

void MainWindow::onRollDiceResult(int result, QString player_name, int player_color) {
    if (this->player_state_ == flychess_game::PlayerState::ROLLING) {
        ui->DiceTextLabel->setText(": " + QString::number(result) + "\n");
        this->player_state_ = flychess_game::PlayerState::SELECTING;
    } else{
        std::string color_str = game_utils::colorIntToString(player_color);
        QString message = QString(" %1 (%2) \n : %3")
                            .arg(player_name)
                            .arg(QString::fromStdString(color_str))
                            .arg(result);
        // ui->DiceTextLabel->setWordWrap(true); // 
        ui->DiceTextLabel->setText(message);
    }
}

void MainWindow::onToRollDice() {
    player_state_ = flychess_game::PlayerState::ROLLING;
    ui->DiceTextLabel->setText("...");
    ui->RollDiceButton->setEnabled(true);
}

void MainWindow::onOtherToRollDice(int color) {
    if (this->player_state_ == flychess_game::PlayerState::ROLLING) return;
    
    std::string color_str = game_utils::colorIntToString(color);
    ui->DiceTextLabel->setText(" " + QString::fromStdString(color_str) + " ...");
}

void MainWindow::onChessCountUpdate(int num) {
    int index = 4 - num;
    if (index >= 0 && index < ui->ChessCountBox->count()) {
        ui->ChessCountBox->setCurrentIndex(index);
    }
}

void MainWindow::onPlayerCountUpdateFailed(int min_num) {
    QMessageBox::warning(this,
                         "",
                         QString(" %1 !").arg(min_num));
}

void MainWindow::onRegisterResult(game_utils::Color color) {
    this->user_color_ = color;
    std::string color_str = game_utils::colorToString(color);
    QString message = QString("system: : %1").arg(QString::fromStdString(color_str));
    ui->RoomListWidget->addItem(message);  
}

void MainWindow::onPlayerCountChanged(int index) {
    //  "3"
    QString text = ui->PlayerCountBox->itemText(index);

    // 
    int playerCount = text.left(text.length() - 1).toInt();

    // qDebug() << ":" << playerCount;
    client_->sendPlayerCount(playerCount);
}

void MainWindow::onChessCountChanged(int index) {
    QString text = ui->ChessCountBox->itemText(index);

    // 
    int chess_count = text.left(text.length() - 1).toInt();

    client_->sendChessCount(chess_count);
}

void MainWindow::onSelectedChessPiece(int id, int color) {
    if (this->player_state_ != flychess_game::PlayerState::SELECTING) {
        std::cout << "" << std::endl;
        return;
    }

    if (color != static_cast<int>(user_color_)) {
        std::cout << "" << std::endl;
        return;
    }

    this->client_->sendChosenChessPiece(id, color);
    this->player_state_ = flychess_game::PlayerState::WAITING;
}


void MainWindow::showAboutDialog() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("");

    // 
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setTextInteractionFlags(Qt::TextBrowserInteraction);

    QString text = R"(
        <h3></h3>
        <p> Qt </p>
        : KezJo<br>
        : 0.1.0<br>
        <a href='https://github.com/Kez-Jo1909/NO_Flychess'> GitHub </a>
    )";

    msgBox.setText(text);

    // 
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
        // 
        chess_pieces_.clear();
    }

    for (const QVariantList& piece : pieces) {
        int id = piece[0].toInt();
        int color = piece[1].toInt();
        int position = piece[2].toInt();
        int player_id = piece[3].toInt();
        chess_pieces_.push_back(flychess_game::ChessPieceInfo(id, static_cast<game_utils::Color>(color), position, player_id));
    }

    // 
    emit AllPieceInfo(pieces);
}

void MainWindow::onPlayerListUpdated(const QList<QVariantList>& players) {
    // 
    ui->RoomTableWidget->setRowCount(0);

    // 
    int row = 0;
    for (const QVariantList& player : players) {
        //  player [name(QString), colorInt(int), is_ready(bool)]
        if (player.size() < 3) continue;

        QString name = player[0].toString();
        int colorInt = player[1].toInt();
        bool isReady = player[2].toBool();

        // 
        ui->RoomTableWidget->insertRow(row);

        // ID 
        ui->RoomTableWidget->setItem(row, 0, new QTableWidgetItem(name));

        // 
        std::string colorStr = game_utils::colorIntToString(colorInt);
        QString colorQStr = QString::fromStdString(colorStr);
        ui->RoomTableWidget->setItem(row, 1, new QTableWidgetItem(colorQStr));

        // 
        ui->RoomTableWidget->setItem(row, 2, new QTableWidgetItem(isReady ? "" : ""));

        row++;
    }
}





void MainWindow::UrlEditEnter() {
    QString url = ui->UrlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "", "URL!");
        return;
    }
    // qDebug() << "URL:" << url;
    auto reply = QMessageBox::question(
        this,
        "",
        "",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        is_server = false;
        // 
        connectingBox_ = new QMessageBox(QMessageBox::Information,
                                        "",
                                        "",
                                        QMessageBox::NoButton,
                                        this);
        connectingBox_->setModal(false);
        connectingBox_->show();

        // 5
        connectTimer_->start(5000);

        client_->connectToServer("ws://" + url.toStdString());
    }
}

void MainWindow::onPreparePageStartButtonClicked() {
    // 
    auto reply = QMessageBox::question(
        this,
        "",
        "",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        server_->GameStart();
        // ui->stackedWidget->setCurrentIndex(4);
    }
}

void MainWindow::onPreparePagePrepareButtonClicked() {
    if(prepared){
        ui->PreparePageStartButton->setText("");
        this->client_->sendGetUnPrepared();
    } else {
        ui->PreparePageStartButton->setText("");
        this->client_->sendGetPrepared();
    }
    prepared = !prepared;
}

void MainWindow::onConnected() {
    // 
    connectTimer_->stop();  

    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }
    // QMessageBox::information(this, "", "");
    if (is_server) {
        ui->PlayerCountBox->setEnabled(true);
        ui->ChessCountBox->setEnabled(true);
        ui->ifCardCheckBox->setEnabled(true);
        ui->ifAiCheckBox->setEnabled(true);
        ui->PreparePageStartButton->setText("");
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
        ui->PreparePageStartButton->setText("");
        disconnect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePagePrepareButtonClicked);
        disconnect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonUserClicked);
        connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePagePrepareButtonClicked);
        connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonUserClicked);
        // ui->stackedWidget->setCurrentIndex(3);
    }
    ui->stackedWidget->setCurrentIndex(3);
    // 
    client_->sendUserInfo("player");

    if(server_) {
        // 
        this->client_->sendGetPrepared();
    }
}

void MainWindow::onDisconnected() {
    connectTimer_->stop(); // 

    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }

    if (!is_server) {
        client_->close();
        QMessageBox::warning(this, "", "\n ");
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::onConnectTimeout() {
    if (connectingBox_) {
        connectingBox_->close();
        connectingBox_ = nullptr;
    }
    QMessageBox::warning(this, "", "");
    //  WebSocket
    client_->disconnectFromServer();
}

void MainWindow::StartButtonClicked() {
    ui->stackedWidget->setCurrentIndex(2); // 
}

void MainWindow::onSettingExitClicked() {
    ui->stackedWidget->setCurrentIndex(0); // 
}

void MainWindow::onCreateGameButtonClicked() {
    is_server = true;

    // ui->stackedWidget->setCurrentIndex(3);

    server_ = std::make_unique<flychess_server::FlychessServer>(8080);

    std::thread([this] {
        bool server_ret = server_->start();

        if (!server_ret) {
            QMetaObject::invokeMethod(this, [this]() {
                QMessageBox::critical(this, "", "");
                ui->stackedWidget->setCurrentIndex(2);
                ui->RoomListWidget->clear();
            }, Qt::QueuedConnection);
        } else {
            // 
            client_->connectToServer("ws://127.0.0.1:8080");
            // game_ = new flychess_game::FlychessGame();

            QMetaObject::invokeMethod(this, [this]() {
                ui->RoomListWidget->addItem("system: ,");
                ui->RoomListWidget->addItem("system: ");
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void MainWindow::onPreparePageExitButtonUserClicked() {
    auto reply = QMessageBox::question(
        this,
        "",
        "",
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
        "",
        "",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (server_) {
            client_->close();
            server_->stop();

            // reset
            QTimer::singleShot(100, this, [this]() {
                server_.reset();
                std::cout << "server stopped." << std::endl;
                ui->RoomListWidget->addItem("system: ");
            });
        }

        ui->stackedWidget->setCurrentIndex(2);   
        ui->RoomListWidget->clear();
    }
}


void MainWindow::onSettingReButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "",
        "",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        
    }
}

void MainWindow::onSettingSaveButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "",
        "",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        
    }
}

void MainWindow::onExitButtonClicked() {
    auto reply = QMessageBox::question(
        this,
        "",
        "",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event); // 

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
    // 
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
//     resize(size, size); // 
//     QWidget::resizeEvent(event);
// }

void ChessBoardWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 
    boardSizePx = std::min(width(), height()) / 34 * 34;
    offsetX = (width()  - boardSizePx) / 2;
    offsetY = (height() - boardSizePx) / 2;

    grid_size = boardSizePx / 17; // 
    radius = boardSizePx / 51;

    // 
    painter.fillRect(rect(), Qt::white);

    // 
    painter.fillRect(QRect(offsetX, offsetY, boardSizePx, boardSizePx), QColor(255, 255, 255));

    // 
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
        // std::cout<<"width:" << grid->width <<std::endl;

        if(type == 0 || type == 2) {
            // 
            int width = grid->width / 40 * grid_size;
            // std::cout<<"type0 size:" << width << std::endl;
            int height = grid->height / 40 * grid_size;
            QRect rect(p_x, p_y, width, height);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
            painter.drawRect(rect);


            // 
            // int center_x = p_x + width / 2;
            // int center_y = p_y + height / 2;
            std::pair<int,int> center_position = getGridCenter(p_x, p_y, width, height, type);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 
        }
        else if(type == 3) {
            p_x = offsetX + boardSizePx / 2;
            p_y = offsetY + boardSizePx / 2;
            int c_x = p_x;
            int c_y = p_y;
            int height = grid->height;
            int width = grid->width / 40.0f * grid_size;// 40.0f * grid_size,float
            // std::cout<<"type3 size:" << width << std::endl;
            painter.setRenderHint(QPainter::Antialiasing); // 
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
            painter.drawPolygon(triangle); // 

            // 
            std::pair<int,int> center_position = getGridCenter(p_x, p_y, width, height, type);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 
        }
        else {
            int height = grid->height;
            int width = grid->width / 40.0f * grid_size;
            painter.setRenderHint(QPainter::Antialiasing); // 
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

            painter.drawPolygon(triangle); // 

            // 
            std::pair<int,int> center_position = getGridCenter(p_x, p_y, width, height, type);
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); 
        }
    }

    // 
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
            // painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 
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
            // painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 
        }
        else {
            int height = grid_info->height;
            int width = grid_info->width / 40.0f * grid_size;
            center_position = getGridCenter(p_x, p_y, width, height, type);
        }
        painter.setPen(QPen(Qt::black, 1));
        // std::vector<int> color_vector = game_utils::colorintToRGB(static_cast<int> (chess_pieces_[i].color));
        painter.setBrush(QColor(color_vector[0], color_vector[1], color_vector[2]));
        painter.drawEllipse(QPoint(center_position.first, center_position.second), radius, radius); // 
    }
}

void ChessBoardWidget::updatePieces(const QList<QVariantList> &pieces) {
    // 
    chess_pieces_.clear();

    // 
    for (const QVariantList &piece : pieces) {
        int id = piece[0].toInt();
        int color = piece[1].toInt();
        int position = piece[2].toInt();
        int player_id = piece[3].toInt();
        chess_pieces_.emplace_back(id, static_cast<game_utils::Color>(color), position, player_id);
    }

    // 
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
    QPoint pos = event->pos(); // 

    // qDebug() << ":" << pos;
    // 
    // int actual_x = pos.x() - offsetX;
    // int actual_y = pos.y() - offsetY;
    // qDebug() << ":" << actual_x << actual_y;

    // 
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
            qDebug() << "ID:" << id << ":" << game_utils::colorToString(picked_color).c_str();
            emit selectedChessPiece(id, static_cast<int>(picked_color));
            return;
        }
    }
}