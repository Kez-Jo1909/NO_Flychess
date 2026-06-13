#ifndef CARD_H
#define CARD_H

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "./utils.h"

namespace flychess_game {

// ============================================================
// flychess_card — 卡牌数据基类（客户端显示用）
// ============================================================
class flychess_card {
public:
    virtual ~flychess_card() = default;

    flychess_card(const nlohmann::json &j) {
        id_ = j.at("id");
        name_ = j.at("name");
        description_ = j.at("description");
        function_time_ = static_cast<game_utils::CardFunctionTime>(j.at("function_time"));
        img_path_ = j.at("image_path");
        target_selection_ = j.value("target_selection", 0);
    }

    int get_id() const { return id_; }
    std::string get_name() const { return name_; }
    std::string get_desc() const { return description_; }
    std::string get_img() const { return img_path_; }
    game_utils::CardFunctionTime get_functiontime() const { return function_time_; }
    int get_target_selection() const { return target_selection_; }

protected:
    int id_;
    std::string name_;
    std::string description_;
    game_utils::CardFunctionTime function_time_;
    std::string img_path_;
    int target_selection_ = 0;
};

// ============================================================
// CardFactory — 卡牌实例工厂（客户端渲染使用）
// ============================================================
class CardFactory {
public:
    using Creator = std::function<std::unique_ptr<flychess_card>(const nlohmann::json&)>;

    static CardFactory& get_instance() {
        static CardFactory factory;
        return factory;
    }

    void register_card(const int card_id, Creator creator) {
        creators[card_id] = std::move(creator);
    }

    std::unique_ptr<flychess_card> create(const nlohmann::json& j) {
        auto id = j.at("id").get<int>();
        auto it = creators.find(id);
        if (it != creators.end()) {
            return it->second(j);
        }
        std::cerr << "[CardFactory] Unknown card id: " << id << std::endl;
        return nullptr;
    }

    size_t registered_count() const {
        return creators.size();
    }

private:
    std::unordered_map<int, Creator> creators;
};

// 初始化：注册所有卡牌的工厂（客户端显示需要）
void register_cards();

// ============================================================
// 卡牌配置加载工具
// ============================================================

// 从 card.json 加载全部卡牌配置
nlohmann::json load_card_config(const std::string& filepath);

// 按 ID 查找单张卡牌的 JSON
nlohmann::json find_card_by_id(const nlohmann::json& card_list, int card_id);

}  // namespace flychess_game

#endif
