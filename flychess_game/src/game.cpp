#include "game.h"
#include <random>
#include <emscripten/bind.h>

int roll_dice() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}

EventType roll_event() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 3); // NONE, KILL, DODGE, DRAW_TWO
    return static_cast<EventType>(dist(gen));
}

EMSCRIPTEN_BINDINGS(game_module) {
    emscripten::function("roll_dice", &roll_dice);
    emscripten::function("roll_event", &roll_event);
    emscripten::enum_<EventType>("EventType")
        .value("NONE", EventType::NONE)
        .value("KILL", EventType::KILL)
        .value("DODGE", EventType::DODGE)
        .value("DRAW_TWO", EventType::DRAW_TWO);
}
