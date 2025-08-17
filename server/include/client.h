#ifndef CLIENT_H
#define CLIENT_H
#pragma once
#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <QVariant>
#include <QString>
#include <QList>
#include <memory>
#include <thread>
#include <chrono>
#include <QObject>
#include <nlohmann/json.hpp>
#include "utils.h"


namespace flychess_client {

class FlychessClient : public QObject {
    Q_OBJECT
public:
    explicit FlychessClient(QObject *parent = nullptr);
    ~FlychessClient();

    void connectToServer(const std::string &url);

    void close();

    void sendMessage(const std::string& message);

    void disconnectFromServer();

    void sendUserInfo(const std::string user_name);

    void sendGetPrepared();
    void sendGetUnPrepared();

    void sendPlayerCount(int num);
    void sendChessCount(int num);

    void sendRollDiceRequest();

    void sendChosenChessPiece(int id, int color);

    void sendFinishUseCard(int color);

    void sendFinishTextWaiting();
signals:
    void connected();
    void disconnected();
    void messageReceived(QString msg);  //  Qt 
    void registerResult(game_utils::Color color);
    void newPlayerJoined(QString name, game_utils::Color color);
    void unknownMessage(QString msg);
    void playerListUpdated(QList<QVariantList> players);
    void playerLeaveRoom(QString name, game_utils::Color color);
    void updatePlayerCount(int new_pc);
    void updatePlayerCountFailed(int min_num);
    void updateChessCount(int new_cc);
    void GameStart();
    void GameStartFailed();
    void GameStartNotEnough();
    void allPieceInfo(QList<QVariantList> pieces);
    void rollDiceResult(int result, QString player_name, int player_color);
    void toRollDice();
    void OtherToRollDice(int color);
    void toUseCard();
    void NoAvailableChess(int color);
    void SomeoneFinished(int color);
    void AllPlayerFinished(QList<QVariantList> rank_list);
private:
    ix::WebSocket ws_;

    game_utils::Color user_color_;
};

}


#endif