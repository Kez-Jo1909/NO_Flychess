#include "../include/card.h"

namespace flychess_game {
    void register_cards() {
        auto& factory = CardFactory::get_instance();

        factory.register_card(0, [](const nlohmann::json& j) { return std::make_unique<Card_Six>(j);});
    }

    void register_functions() {
        auto& reg = CardFunctionRegistery::get_instance();

        reg.register_function(0, Card_Six::function);
    }

    void Card_ExtremeWeather::function(FlychessGame &game, int player_id, int target_player_id) {
        // TODO
    }
}