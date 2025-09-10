#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QUrl>
#include <QTimer>
#include <QDesktopServices>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QVariant>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QList>
#include <QComboBox>
#include "game.h"
#include "card.h"
#include "server.h"
#include "client.h"
#include "utils.h"


#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

std::string getLocalIP();

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


namespace flychess_client{

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void StartButtonClicked();  // 开始按钮槽函数

    void onExitButtonClicked();// 退出按钮槽函数

    void onSettingButtonClicked();// 设置按钮槽函数

    void onSettingExitClicked();// 设置中返回槽函数

    void onSettingReButtonClicked();// 设置中重置按钮槽函数

    void onSettingSaveButtonClicked();

    void showAboutDialog();
    void showUpdateDialog();

    void onCreateGameButtonClicked();

    void onRollDiceButton();
    void onRollDiceResult(int result, QString player_name, int player_color);

    void onPreparePageExitButtonClicked();
    void onPreparePageExitButtonUserClicked();

    void UrlEditEnter();

    void onPreparePageStartButtonClicked();
    void onPreparePagePrepareButtonClicked();

    void onToRollDice();
    void onOtherToRollDice(int color);

    void onPlayerCountChanged(int index);
    void onChessCountChanged(int index);

    void onNewPlayerJoined(QString name, game_utils::Color color);
    void onRegisterResult(game_utils::Color color);
    void onPlayerListUpdated(const QList<QVariantList>& players);
    void onAllPieceInfo(const QList<QVariantList>& pieces);
    void onAllPlayerFinished(const QList<QVariantList>& rank_list);
    void onPlayerLeaveRoom(QString name, game_utils::Color color);
    void onPlayerCountUpdate(int num);
    void onChessCountUpdate(int num);
    void onPlayerCountUpdateFailed(int min_num);
    void onGameStart();
    void onGameStartFailed();
    void onGameStartNotEnough();
    void onSelectedChessPiece(int id, int color);
    void onToUseCard();
    void onNoAvailableChess(int color, int dice_num);
    void onSomeoneFinished(int color);
    void onURLReceived(QString url);

    void onJoinGameButtonClicked();
    void ChatEditEnter();
    void onChatMessageRecieved(QString name, QString message, int color);

    void onGetNewCard(int card_id);
    void onCardClicked(int card_id);
private:
    // void repositionStartMenu();

    void onConnected();
    void onDisconnected();
    void onConnectTimeout();

signals:
    void AllPieceInfo(QList<QVariantList> pieces);

protected:
    void resizeEvent(QResizeEvent *event) override;

// 以下是私有成员变量
private:
    Ui::MainWindow *ui;
    std::unique_ptr<flychess_server::FlychessServer> server_;

    // 直接在类内声明客户端实例
    flychess_client::FlychessClient *client_;

    QMessageBox* connectingBox_ = nullptr;  // “正在连接”提示框
    QTimer* connectTimer_ = nullptr;        // 连接超时定时器

    bool is_server = false;
    bool prepared = false;

    flychess_game::PlayerState player_state_ = flychess_game::PlayerState::UNDEFINED;

    game_utils::Color user_color_ = game_utils::Color::UNDEFINED; // 默认颜色

    std::vector<flychess_game::ChessPieceInfo> chess_pieces_; // 棋子信息列表

    std::string version = "0.2.3";
};

}// namespace flychess_client

class ChessBoardWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChessBoardWidget(QWidget *parent = nullptr);
public slots:
    void updatePieces(const QList<QVariantList> &pieces);
signals:
    void selectedChessPiece(int id, int color);
protected:
    // void resizeEvent(QResizeEvent *event) override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    std::pair<int,int> getGridCenter(int px, int py, int width, int height, int type);
private:
    std::vector<flychess_game::ChessPieceInfo> chess_pieces_front_; // 绘制使用
    std::vector<flychess_game::ChessPieceInfo> chess_pieces_back_;  // 更新使用
    std::mutex pieces_mutex_;

    int boardSizePx;
    int offsetX;
    int offsetY;
    int radius;
    int grid_size;
};

class CardItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal scaleFactor READ scaleFactor WRITE setScaleFactor)

public:
    CardItem(const QString &img_path, int card_id, QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent), card_id(card_id), img_path(img_path)
    {
        originalPixmap = QPixmap(img_path);
        aspectRatio = originalPixmap.isNull() ? (2.0 / 3.0) 
                                              : double(originalPixmap.width()) / originalPixmap.height();
        cardHeight = 180;
        scaleFactor_ = 1.0;
        updatePixmap();
        setFlag(QGraphicsItem::ItemIsSelectable);
        setAcceptHoverEvents(true);
    }

    ~CardItem() override {
        qDebug() << "[Debug] CardItem destroyed:" << card_id;
    }

    QRectF boundingRect() const override {
        return QRectF(0, 0, cardWidth * scaleFactor_, cardHeight * scaleFactor_);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        if (!pixmap.isNull()) {
            painter->drawPixmap(0, 0, pixmap);
        }
    }

    int getCardId() const { return card_id; }

    void setCardHeight(int height) {
        prepareGeometryChange();  // 几何改变前调用
        cardHeight = height;
        cardWidth = int(cardHeight * aspectRatio);
        updatePixmap();
        update();
    }

    qreal scaleFactor() const {
        qDebug() << "[Debug] scaleFactor getter called for" << card_id;
        return scaleFactor_;
    }

    void setScaleFactor(qreal factor) {
        if (qFuzzyCompare(scaleFactor_, factor)) return;
        prepareGeometryChange();  // 修改前调用
        qDebug() << "[Debug] setScaleFactor called for" << card_id << "factor=" << factor;
        scaleFactor_ = factor;
        updatePixmap();
        update();
    }

signals:
    void cardClicked(int card_id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit cardClicked(card_id);
        }
        QGraphicsObject::mousePressEvent(event);
    }

    // 已删除 hoverEnterEvent / hoverLeaveEvent 动画逻辑

private:
    int card_id = -1;
    QString img_path;
    QPixmap originalPixmap;
    QPixmap pixmap;
    int cardWidth = 120;
    int cardHeight = 180;
    double aspectRatio = 2.0 / 3.0;
    qreal scaleFactor_ = 1.0;

    void updatePixmap() {
        if (!originalPixmap.isNull()) {
            int w = qMax(1, int(cardWidth * scaleFactor_));
            int h = qMax(1, int(cardHeight * scaleFactor_));
            pixmap = originalPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
};



class CardView : public QGraphicsView {
    Q_OBJECT
public:
    explicit CardView(QWidget* parent = nullptr)
        : QGraphicsView(parent)
    {
        scene = new QGraphicsScene(this);
        setScene(scene);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        setBackgroundBrush(Qt::white);

        // 禁用滚动条
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // 场景大小跟随视口
        scene->setSceneRect(this->viewport()->rect());
    }


    void addCard(const QString& img_path, int card_id) {
        CardItem* card = new CardItem(img_path, card_id);
        scene->addItem(card);
        cardItems.append(card);
        connect(card, &CardItem::cardClicked, this, &CardView::cardClicked);
        layoutCards();
    }

    void removeCard(int card_id) {
        for (int i = 0; i < cardItems.size(); ++i) {
            if (cardItems[i]->getCardId() == card_id) {
                scene->removeItem(cardItems[i]);
                delete cardItems[i];
                cardItems.remove(i);
                break;
            }
        }
        layoutCards();
    }

    // 可一次性设置全部卡牌
    void setCards(const QVector<QPair<QString, int>>& cards) {
        // 清空
        for (auto card : cardItems) {
            scene->removeItem(card);
            delete card;
        }
        cardItems.clear();
        // 添加
        for (const auto& pair : cards) {
            addCard(pair.first, pair.second);
        }
        layoutCards();
    }

signals:
    void cardClicked(int card_id);

protected:
    void resizeEvent(QResizeEvent* event) override {
        QGraphicsView::resizeEvent(event);
        scene->setSceneRect(rect());
        layoutCards();
    }

private:
    QGraphicsScene* scene;
    QVector<CardItem*> cardItems;

    void layoutCards() {
        if (cardItems.isEmpty()) return;

        int n = cardItems.size();
        int spacing = 10;

        int viewWidth = this->viewport()->width();
        int viewHeight = this->viewport()->height();

        // 默认高度（不超过视口高度）
        int defaultHeight = viewHeight - 20; 
        if (defaultHeight < 50) defaultHeight = 50; // 不要太小

        // 先用默认高度算一次宽度
        for (auto card : cardItems) {
            card->setCardHeight(defaultHeight);
        }
        int cardWidth = int(cardItems[0]->boundingRect().width());

        int totalWidth = n * cardWidth + (n - 1) * spacing;

        // 如果太宽 -> 缩放
        double scaleFactor = 1.0;
        if (totalWidth > viewWidth) {
            scaleFactor = double(viewWidth - (n - 1) * spacing) / (n * cardWidth);
            int newHeight = int(defaultHeight * scaleFactor);
            for (auto card : cardItems) {
                card->setCardHeight(newHeight);
            }
            cardWidth = int(cardItems[0]->boundingRect().width());
            totalWidth = n * cardWidth + (n - 1) * spacing;
        }

        // 居中放置
        int x0 = (viewWidth - totalWidth) / 2;
        int y0 = (viewHeight - cardItems[0]->boundingRect().height()) / 2;

        for (int i = 0; i < n; ++i) {
            cardItems[i]->setPos(x0 + i * (cardWidth + spacing), y0);
        }
    }

};


#endif // MAINWINDOW_H
