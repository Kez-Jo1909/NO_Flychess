# 卡牌设计规范

## 卡牌数据模型

每张卡牌在 `config/card.json` 中定义，包含以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `id` | int | ✓ | 卡牌唯一 ID（与 C++ 注册 ID 对应） |
| `name` | string | ✓ | 卡牌名称 |
| `description` | string | ✓ | 效果描述文字 |
| `function_time` | int | ✓ | 使用时机（见下表） |
| `image_path` | string | ✓ | 卡面图片路径 |
| `target_selection` | int | ✓ | 目标选择类型（见下表） |
| `effects` | array | ✓ | 效果指令序列（DSL） |

### function_time 枚举

| 值 | 含义 | 说明 |
|----|------|------|
| 0 | `ANYTIME` | 任意时刻可用（响应型卡牌） |
| 1 | `YOUR_TURN` | 仅自己回合 |
| 2 | `BEFORE_ROLL` | 掷骰子前 |
| 3 | `BEFORE_MOVE` | 移动棋子前 |
| 4 | `AFTER_MOVE` | 移动棋子后 |

### target_selection 枚举

| 值 | 含义 | 客户端 UI 行为 |
|----|------|---------------|
| 0 | 无需目标 | 直接使用 |
| 1 | 选择玩家 | 弹出玩家选择器 |
| 2 | 选择棋子 | 弹出棋子选择器（需先选玩家再选棋子） |

## 现有卡牌

### 卡牌 0: "6"

```
效果: 使自己投出一个 6
时机: BEFORE_ROLL（掷骰子前）
目标: 无需选择
```

```json
{
  "id": 0,
  "name": "6",
  "description": "使自己投出一个6",
  "function_time": 2,
  "image_path": "../assets/cards/6.jpg",
  "target_selection": 0,
  "effects": [
    {"op": "set_dice", "value": 6}
  ]
}
```

### 卡牌 1: "极端天气"

```
效果: 所有处于待飞区的飞机返回重生点
时机: ANYTIME（任意时刻）
目标: 无需选择
```

```json
{
  "id": 1,
  "name": "极端天气",
  "description": "使用后使所有处于待飞区的飞机返回重生点",
  "function_time": 0,
  "image_path": "../assets/cards/extreme_weather.jpg",
  "target_selection": 0,
  "effects": [
    {"op": "send_all_pre_to_start", "player": "$caster"}
  ]
}
```

## DSL 原子指令参考

### set_dice — 强制骰子点数

```json
{"op": "set_dice", "value": <int>}
```

设置下次掷骰子的点数。`value=6` 等效于"必出 6"。

### send_to_start — 单体送回起点

```json
{"op": "send_to_start", "player": "<color>", "piece_id": <int>}
```

将指定玩家的指定棋子送回重生点（position = -1）。

### send_all_pre_to_start — 群体送回起点

```json
{"op": "send_all_pre_to_start", "player": "<color>"}
```

将指定玩家所有处于待飞区（position == 0）的棋子送回重生点。

### move_piece — 移动棋子

```json
{"op": "move_piece", "player": "<color>", "piece_id": <int>, "steps": <int>}
```

向前移动棋子指定步数。会触发踩子判定。

### fly_piece — 飞行棋子

```json
{"op": "fly_piece", "player": "<color>", "piece_id": <int>}
```

如果棋子落在可飞行的格子上，飞往对称点。会触发踩子判定。

## 如何添加新卡牌

### 简单卡牌（仅需 JSON）

1. 设计卡牌效果，用原子指令组合
2. 在 `config/card.json` 中添加条目
3. 放入卡面图片到 `assets/cards/`
4. 无需重新编译 C++

示例 —— 添加"导弹"卡（选择一个棋子送回起点）：

```json
{
  "id": 2,
  "name": "导弹",
  "description": "选择场上任意一个棋子，将其送回重生点",
  "function_time": 0,
  "image_path": "../assets/cards/missile.jpg",
  "target_selection": 2,
  "effects": [
    {"op": "send_to_start", "player": "$target", "piece_id": "$selected_piece"}
  ]
}
```

### 复杂卡牌（需 C++）

如果卡牌效果无法用现有原子指令组合（如需要条件分支、多阶段交互、特殊 UI），则：

1. 在 `flychess_game/include/command.h` 中新增 ICommand 子类
2. 在 `flychess_game/src/card.cpp` 中注册 `"custom"` 工厂
3. 在 `card.json` 中使用 `{"op": "custom", "class": "YourCommandClass"}`

示例 —— 条件卡牌（如果目标有已起飞棋子则送回，否则自己前进 3 步）：

```cpp
// command.h
class ConditionalStrikeCommand : public ICommand {
    int caster_, target_;
public:
    ConditionalStrikeCommand(int caster, int target) : caster_(caster), target_(target) {}
    void execute(FlychessGame& game) override {
        if (game.GetStartedChessCount(target_) > 0) {
            // 目标有已起飞棋子，送回去
            auto& player = game.GetPlayer(target_);
            for (int i = 0; i < player.GetChessPieceCount(); i++) {
                if (player.GetChessPieceInfo(i).position > 0) {
                    player.SendChessPieceBackHome(i);
                    break;
                }
            }
        } else {
            // 目标没有已起飞棋子，自己前进 3 步
            game.GetPlayer(caster_).MoveChessPiece(0, 3);
        }
    }
    std::string describe() const override { return "条件打击"; }
};
```

## 卡牌设计原则

### 平衡性指南

- **抽卡频率**: 当前掷出 5 才抽卡（概率 1/6）。如果卡池增大，考虑调整为独立抽卡阶段
- **卡牌稀有度**: 未来可以加 `rarity` 字段，抽卡时按权重随机
- **上限控制**: 同种卡牌数量上限 4 张（代码已有注释但未启用）

### 交互设计

- 无目标卡牌：客户端选中后立即生效
- 有目标卡牌：客户端先弹出目标选择 UI，确认后发送
- 响应型卡牌（`function_time: 0`）：可在他人的卡牌结算中插入（需要客户端做"是否响应"提示）

### 卡牌分类建议

| 类别 | 时机 | 典型效果 | 示例 |
|------|------|---------|------|
| 增益 | BEFORE_ROLL | 操纵骰子点数 | "6" |
| 攻击 | ANYTIME | 干扰对手棋子 | "极端天气"、"导弹" |
| 移动 | BEFORE_MOVE | 额外移动步数 | "加速"（+2 步） |
| 防御 | ANYTIME | 保护棋子 | "护盾"（免疫一次踩子） |
| 特殊 | ANYTIME | 改变规则 | "双骰"（掷两次选一个） |

## 待设计卡牌池

以下是可以考虑添加的卡牌（仅思路，未实现）：

| ID | 暂定名 | 效果 | 指令 |
|----|--------|------|------|
| 2 | 导弹 | 选择一个棋子送回起点 | send_to_start |
| 3 | 加速 | 本次移动 +2 步 | move_piece（steps 参数拼接） |
| 4 | 护盾 | 免疫下一次被踩 | 需要新指令 + 状态标记 |
| 5 | 交换 | 与自己另一个棋子交换位置 | 需要新指令 |
| 6 | 召唤 | 直接让一个棋子起飞（掷 6 效果） | move_piece 从 home 到 start |
| 7 | 冰冻 | 选择一个玩家，跳过其下回合 | 需要新指令 + 状态标记 |
