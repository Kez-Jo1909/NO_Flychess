#include "../include/command.h"
#include "../include/game.h"
#include "../include/player.h"

namespace flychess_game {

// ============================================================
// SetDiceCommand
// ============================================================
void SetDiceCommand::execute(FlychessGame& game) {
    // 直接设置骰子点数（不再对 6 做特殊处理）
    game.setDice(value_);
}

// ============================================================
// SendToStartCommand
// ============================================================
void SendToStartCommand::execute(FlychessGame& game) {
    auto& player = game.GetPlayer(player_);
    player.SendChessPieceBackHome(piece_id_);
}

// ============================================================
// SendAllPreToStartCommand
// ============================================================
void SendAllPreToStartCommand::execute(FlychessGame& game) {
    game.setAllPreBack(player_);
}

// ============================================================
// MovePieceCommand
// ============================================================
void MovePieceCommand::execute(FlychessGame& game) {
    int ret = game.MoveChessPiece(player_, piece_id_, steps_);
    if (ret <= 0) {
        std::cerr << "[MovePieceCommand] 移动失败: player=" << player_
                  << " piece=" << piece_id_ << " steps=" << steps_
                  << " ret=" << ret << std::endl;
    }
}

// ============================================================
// FlyPieceCommand
// ============================================================
void FlyPieceCommand::execute(FlychessGame& game) {
    int ret = game.FlyChessPiece(player_, piece_id_);
    if (ret > 0) {
        std::cout << "[FlyPieceCommand] 飞行成功: player=" << player_
                  << " piece=" << piece_id_ << std::endl;
    }
}

// ============================================================
// DSL 解析
// ============================================================

int resolve_variable(const nlohmann::json& val, const std::string& key,
                     int caster, int target, int piece) {
    // 如果值已经是整数，直接返回
    if (val.is_number_integer()) {
        return val.get<int>();
    }
    // 如果是字符串，检查是否为变量占位符
    if (val.is_string()) {
        std::string s = val.get<std::string>();
        if (s == "$caster")        return caster;
        if (s == "$target")        return target;
        if (s == "$selected_piece") return piece;
        // 普通字符串尝试转为整数
        try {
            return std::stoi(s);
        } catch (...) {
            std::cerr << "[DSL] 无法解析变量: " << s << std::endl;
            return -1;
        }
    }
    std::cerr << "[DSL] 未知的值类型 for key='" << key << "'" << std::endl;
    return -1;
}

std::unique_ptr<ICommand> create_command(const nlohmann::json& effect_json,
                                         int caster, int target, int piece) {
    std::string op = effect_json.at("op").get<std::string>();

    if (op == "set_dice") {
        int value = resolve_variable(effect_json.at("value"), "value", caster, target, piece);
        return std::make_unique<SetDiceCommand>(value);
    }
    else if (op == "send_to_start") {
        int player = resolve_variable(effect_json.at("player"), "player", caster, target, piece);
        int piece_id = resolve_variable(effect_json.at("piece_id"), "piece_id", caster, target, piece);
        return std::make_unique<SendToStartCommand>(player, piece_id);
    }
    else if (op == "send_all_pre_to_start") {
        int player = resolve_variable(effect_json.at("player"), "player", caster, target, piece);
        return std::make_unique<SendAllPreToStartCommand>(player);
    }
    else if (op == "move_piece") {
        int player   = resolve_variable(effect_json.at("player"), "player", caster, target, piece);
        int piece_id = resolve_variable(effect_json.at("piece_id"), "piece_id", caster, target, piece);
        int steps    = resolve_variable(effect_json.at("steps"), "steps", caster, target, piece);
        return std::make_unique<MovePieceCommand>(player, piece_id, steps);
    }
    else if (op == "fly_piece") {
        int player   = resolve_variable(effect_json.at("player"), "player", caster, target, piece);
        int piece_id = resolve_variable(effect_json.at("piece_id"), "piece_id", caster, target, piece);
        return std::make_unique<FlyPieceCommand>(player, piece_id);
    }
    else if (op == "custom") {
        std::string class_name = effect_json.at("class").get<std::string>();
        return CustomCommandRegistry::get_instance().create(class_name, effect_json, caster, target, piece);
    }
    else {
        std::cerr << "[DSL] 未知的 op: " << op << std::endl;
        return nullptr;
    }
}

CardEffect parse_card_effect(const nlohmann::json& card_json,
                             int caster, int target, int piece) {
    CardEffect effect;
    effect.card_id   = card_json.at("id").get<int>();
    effect.card_name = card_json.at("name").get<std::string>();
    effect.source_player = caster;
    effect.target_player = target;
    effect.target_piece  = piece;

    if (card_json.contains("effects") && card_json["effects"].is_array()) {
        for (const auto& e : card_json["effects"]) {
            auto cmd = create_command(e, caster, target, piece);
            if (cmd) {
                effect.commands.push_back(std::move(cmd));
            }
        }
    } else {
        std::cerr << "[DSL] 卡牌 " << effect.card_name
                  << " (id=" << effect.card_id << ") 没有 effects 数组" << std::endl;
    }

    return effect;
}

}  // namespace flychess_game
