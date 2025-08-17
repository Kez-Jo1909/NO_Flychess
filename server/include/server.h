#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "game.h"
#include "utils.h"

namespace flychess_server{

class FlychessServer : public QObject {
    Q_OBJECT
public:
    explicit FlychessServer(int port = 8080, QObject* parent = nullptr);

    bool start();                     // 
    void stop();                       // 
    void handleMessage(const ix::WebSocketMessagePtr& msg, const std::string& client_id);

    void GameStart();

signals:
    void clientConnected(const QString& client_id, const QString& ip);
    void clientDisconnected(const QString& client_id);
    void messageReceived(const QString& client_id, const QString& msg);
    void jsonReceived(const QString& client_id, const QJsonObject& json);

private:
    int port_;
    std::unique_ptr<ix::WebSocketServer> server_;
    std::unordered_map<std::string, std::shared_ptr<ix::WebSocket>> clients_;// client_id -> WebSocket

    void setupMessageCallback(std::shared_ptr<ix::WebSocket> webSocket, const std::string& client_id);
    void sendDiceNum(int dice_num_, const std::string& client_id);
    void sendToClient(const std::string& client_id, const std::string msg);
    void BroadCast(const std::string& msg);

    void BroadCastPlayerList();

    void CardState(const std::string client_id);

    void BroadCastPlayerCount();
    void BroadCastChessCount();
    void BroadCastRoomInfo();
    void BroadCastPieceInfo(int player_count, int cp_count);
    void BroadCastToRollDice(int color_to_roll);
    void BroadCastSomeoneFinished(int color_finished);
    void BroadCastAllFinished();

    flychess_game::FlychessGame *game_; // 
    flychess_game::FlychessGameRoom *game_room_; // 

    // int finished_player_count = 0;
    std::vector<int> finished_players;

    int steps = -1;
};

}

#endif
