#include "../include/card.h"

namespace flychess_game {
    void register_cards() {
        auto& factory = CardFactory::get_instance();

        factory.register_card(0, [](const nlohmann::json& j) { return std::make_unique<Card_Six>(j);});
        factory.register_card(1, [](const nlohmann::json& j) { return std::make_unique<Card_ExtremeWeather>(j);});
    }

    void register_functions() {
        auto& reg = CardFunctionRegistery::get_instance();

        reg.register_function(0, Card_Six::function);
        reg.register_function(1, Card_ExtremeWeather::function);
    }

    void Card_ExtremeWeather::function(FlychessGame &game, int player_id, int target_player_id) {
        // TODO
        std::cout << "Card_ExtremeWeather function executed!" << std::endl;
        game.setAllPreBack(player_id);
        std::cout<< "[Debug] Card_ExtremeWeather function end" << std::endl;
    }
}