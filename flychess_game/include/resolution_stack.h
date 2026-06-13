#ifndef RESOLUTION_STACK_H
#define RESOLUTION_STACK_H

#include <vector>
#include <memory>
#include <iostream>
#include "command.h"

namespace flychess_game {

// ============================================================
// ResolutionStack — 后进先出的卡牌结算栈
//
// 用法：
//   1. 打出卡牌 → parse_card_effect() → pushEffect(effect)
//   2. resolveAll(game) 从栈顶依次执行
//   3. 结算中若触发新卡牌，pushEffect 压入栈顶，后发先至
// ============================================================
class ResolutionStack {
public:
    ResolutionStack() = default;

    // 压入单条指令
    void push(std::unique_ptr<ICommand> cmd) {
        stack_.push_back(std::move(cmd));
    }

    // 压入整张卡牌的效果序列（逆序压入以保证正序执行）
    void pushEffect(CardEffect& effect) {
        std::cout << "[ResolutionStack] 压入卡牌效果: " << effect.card_name
                  << " (" << effect.commands.size() << " 条指令)" << std::endl;
        // 逆序遍历：先压入的最后执行
        for (auto it = effect.commands.rbegin(); it != effect.commands.rend(); ++it) {
            if (*it) {
                stack_.push_back(std::move(*it));
            }
        }
        // 清空原 vector（所有权已转移）
        effect.commands.clear();
    }

    // 栈顶 → 栈底依次执行所有指令
    void resolveAll(FlychessGame& game) {
        int count = 0;
        while (!stack_.empty()) {
            auto cmd = std::move(stack_.back());
            stack_.pop_back();

            if (cmd) {
                std::cout << "[ResolutionStack] 执行(" << count << "): "
                          << cmd->describe() << std::endl;
                cmd->execute(game);
                count++;
            }
        }
        std::cout << "[ResolutionStack] 结算完毕，共执行 " << count << " 条指令" << std::endl;
    }

    bool isEmpty() const {
        return stack_.empty();
    }

    size_t size() const {
        return stack_.size();
    }

    void clear() {
        stack_.clear();
    }

private:
    // back = 栈顶（最后压入，最先执行）
    std::vector<std::unique_ptr<ICommand>> stack_;
};

}  // namespace flychess_game

#endif
