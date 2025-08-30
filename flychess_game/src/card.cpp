#include "../include/card.h"

namespace flychess_game {
    void register_cards() {
        auto& factory = CardFactory::get_instance();
        factory.register_card(0, [](const nlohmann::json& j) { return std::make_unique<Card_Six>(j);});
    }
}