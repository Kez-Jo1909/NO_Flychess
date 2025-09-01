#ifndef CARD_H
#define CARD_H

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "./utils.h"
#include "./game.h"

namespace flychess_game {

class flychess_card {
public:
    virtual ~flychess_card() = default;

    flychess_card(const nlohmann::json &j) {
        id_ = j.at("id");
        name_ = j.at("name");
        description_ = j.at("description");
        function_time_ = static_cast<game_utils::CardFunctionTime>(j.at("function_time"));
        img_path_ = j.at("image_path");
    }

    int get_id() const { return id_; }
    std::string get_name() const { return name_; }
    std::string get_desc() const { return description_; }
    std::string get_img() const { return img_path_; }
    game_utils::CardFunctionTime get_time() const { return function_time_; }
protected:
    int id_;
    std::string name_;
    std::string description_;
    game_utils::CardFunctionTime function_time_;
    std::string img_path_;
};

class Card_Six : public flychess_card {
public:
    using flychess_card::flychess_card; // Inherit constructor
    // Card_Six(const nlohmann::json &j) : flychess_card(j) {
    //     // Additional initialization if needed
    // }

    static void function(FlychessGame& game, int player_id, int target_player_id) {
        std::cout << "Card_Six function executed!" << std::endl;
        game.setDice(6);
    }
};


class CardFactory {
public:
    using Creator = std::function<std::unique_ptr<flychess_card>(const nlohmann::json&)>;

    static CardFactory& get_instance() {
        static CardFactory factory;
        return factory;
    }

    void register_card(const int card_id, Creator creator) {
        creators[card_id] = creator;
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

private:
    std::unordered_map<int, Creator> creators;

};

using CardFunction = std::function<void(FlychessGame& game, int player_id, int target_player_id)>;

class CardFunctionRegistery {
public:
    static CardFunctionRegistery& get_instance() {
        static CardFunctionRegistery instance;
        return instance;
    }

    void register_function(int card_id, CardFunction func) {
        effects_[card_id] = func;
    }

    void applyFunction(int card_id, FlychessGame& game, int player_id, int target_player_id) {
        auto it = effects_.find(card_id);
        if (it != effects_.end()) {
            it->second(game, player_id, target_player_id);
        } else {
            std::cerr << "[CardFunctionRegistery] No function registered for card id: " << card_id << std::endl;
        }
    }
private:
    std::unordered_map<int, CardFunction> effects_;
};

void register_cards();
void register_functions();

}

#endif