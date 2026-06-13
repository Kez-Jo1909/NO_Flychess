#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

namespace flychess_game {

// 前向声明
class FlychessGame;

// ============================================================
// ICommand — 原子指令接口
// ============================================================
struct ICommand {
    virtual ~ICommand() = default;
    virtual void execute(FlychessGame& game) = 0;
    virtual std::string describe() const = 0;
};

// ============================================================
// 具体原子指令
// ============================================================

// 强制下次骰子点数为指定值。value=6 等效于 "必出6"
class SetDiceCommand : public ICommand {
public:
    explicit SetDiceCommand(int value) : value_(value) {}

    void execute(FlychessGame& game) override;
    std::string describe() const override {
        return "SetDice(" + std::to_string(value_) + ")";
    }
private:
    int value_;
};

// 将指定玩家的指定棋子送回重生点 (position = -1)
class SendToStartCommand : public ICommand {
public:
    SendToStartCommand(int player, int piece_id)
        : player_(player), piece_id_(piece_id) {}

    void execute(FlychessGame& game) override;
    std::string describe() const override {
        return "SendToStart(player=" + std::to_string(player_)
            + ", piece=" + std::to_string(piece_id_) + ")";
    }
private:
    int player_;
    int piece_id_;
};

// 将指定玩家所有待飞区（position == 0）棋子送回重生点
class SendAllPreToStartCommand : public ICommand {
public:
    explicit SendAllPreToStartCommand(int player) : player_(player) {}

    void execute(FlychessGame& game) override;
    std::string describe() const override {
        return "SendAllPreToStart(player=" + std::to_string(player_) + ")";
    }
private:
    int player_;
};

// 向前移动棋子指定步数（会触发踩子判定）
class MovePieceCommand : public ICommand {
public:
    MovePieceCommand(int player, int piece_id, int steps)
        : player_(player), piece_id_(piece_id), steps_(steps) {}

    void execute(FlychessGame& game) override;
    std::string describe() const override {
        return "MovePiece(player=" + std::to_string(player_)
            + ", piece=" + std::to_string(piece_id_)
            + ", steps=" + std::to_string(steps_) + ")";
    }
private:
    int player_;
    int piece_id_;
    int steps_;
};

// 如果棋子落在可飞行格，飞往对称点
class FlyPieceCommand : public ICommand {
public:
    FlyPieceCommand(int player, int piece_id)
        : player_(player), piece_id_(piece_id) {}

    void execute(FlychessGame& game) override;
    std::string describe() const override {
        return "FlyPiece(player=" + std::to_string(player_)
            + ", piece=" + std::to_string(piece_id_) + ")";
    }
private:
    int player_;
    int piece_id_;
};

// ============================================================
// CardEffect — 卡牌效果（一或多个 ICommand 的序列）
// ============================================================
struct CardEffect {
    int card_id;
    std::string card_name;
    int source_player;        // $caster
    int target_player;        // $target（-1 表示无目标）
    int target_piece;         // $selected_piece（-1 表示无目标）
    std::vector<std::unique_ptr<ICommand>> commands;
};

// ============================================================
// DSL 解析 — 从 JSON effects 数组创建 CardEffect
// ============================================================

// 变量替换：将 "$caster" / "$target" / "$selected_piece" 替换为实际值
int resolve_variable(const nlohmann::json& val, const std::string& key,
                     int caster, int target, int piece);

// 从单个 effect JSON 对象创建 ICommand
std::unique_ptr<ICommand> create_command(const nlohmann::json& effect_json,
                                         int caster, int target, int piece);

// 从卡牌 JSON + 运行时参数创建 CardEffect
CardEffect parse_card_effect(const nlohmann::json& card_json,
                             int caster, int target, int piece);

// ============================================================
// Custom Command 工厂（逃生舱：复杂卡牌走 C++ 子类）
// ============================================================
using CustomCommandCreator = std::function<std::unique_ptr<ICommand>(
    const nlohmann::json&, int caster, int target, int piece)>;

class CustomCommandRegistry {
public:
    static CustomCommandRegistry& get_instance() {
        static CustomCommandRegistry instance;
        return instance;
    }

    void register_class(const std::string& class_name, CustomCommandCreator creator) {
        creators_[class_name] = std::move(creator);
    }

    std::unique_ptr<ICommand> create(const std::string& class_name,
                                     const nlohmann::json& j,
                                     int caster, int target, int piece) {
        auto it = creators_.find(class_name);
        if (it != creators_.end()) {
            return it->second(j, caster, target, piece);
        }
        std::cerr << "[CustomCommandRegistry] Unknown class: " << class_name << std::endl;
        return nullptr;
    }

private:
    std::unordered_map<std::string, CustomCommandCreator> creators_;
};

}  // namespace flychess_game

#endif
