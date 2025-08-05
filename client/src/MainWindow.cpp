#include "../include/MainWindow.h"
#include "game.h"
#include "ui_MainWindow.h"
#include "utils.h"
#include <QDebug>

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

    connect(client_, &FlychessClient::connected,    this, &MainWindow::onConnected);
    connect(client_, &FlychessClient::disconnected, this, &MainWindow::onDisconnected);

    // -
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    connect(client_, &FlychessClient::newPlayerJoined, this, &MainWindow::onNewPlayerJoined);
    connect(client_, &FlychessClient::registerResult, this, &MainWindow::onRegisterResult);
    connect(client_, &FlychessClient::playerListUpdated, this, &MainWindow::onPlayerListUpdated);
    connect(client_, &FlychessClient::playerLeaveRoom, this, &MainWindow::onPlayerLeaveRoom);
    connect(client_, &FlychessClient::updatePlayerCount, this, &MainWindow::onPlayerCountUpdate);
    connect(client_, &FlychessClient::updateChessCount, this, &MainWindow::onChessCountUpdate);
    connect(client_, &FlychessClient::updatePlayerCountFailed, this, &MainWindow::onPlayerCountUpdateFailed);

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
        // TODO : 
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
        disconnect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageStartButtonClicked);
        connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePageStartButtonClicked);
        connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonClicked);
        ui->stackedWidget->setCurrentIndex(3);
    } else {
        ui->PlayerCountBox->setEnabled(false);
        ui->ChessCountBox->setEnabled(false);
        ui->ifCardCheckBox->setEnabled(false);
        ui->ifAiCheckBox->setEnabled(false);
        ui->PreparePageStartButton->setText("");
        disconnect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePagePrepareButtonClicked);
        disconnect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonUserClicked);
        connect(ui->PreparePageStartButton, &QPushButton::clicked, this, &MainWindow::onPreparePagePrepareButtonClicked);
        connect(ui->PreparePageExitButton, &QPushButton::clicked, this, &MainWindow::onPreparePageExitButtonUserClicked);
        ui->stackedWidget->setCurrentIndex(3);
    }
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
}

void MainWindow::onSettingButtonClicked() {
    // 
    ui->stackedWidget->setCurrentIndex(1);
}

}// namespace flychess_client
