#pragma once
#include <string>

enum EventType {
    NONE,
    KILL,
    DODGE,
    DRAW_TWO
};

struct Player {
    std::string name;
    int position = 0;
};

EventType roll_event(); // 随机触发一个锦囊事件
int roll_dice();        // 掷骰子函数
