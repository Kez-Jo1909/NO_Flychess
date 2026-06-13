# ResolutionStack 卡牌结算系统设计

## 概述

将卡牌效果从"直接函数调用"改为"指令压栈 → 栈顶优先结算"的模式。灵感来源于 MTG 的堆叠（Stack）和游戏王的连锁（Chain）。

## 核心概念

```
打出卡牌 → 编译为 CardEffect → 压入 ResolutionStack
                                  ↓
触发连锁 → 新效果压入栈顶        resolveAll() 从栈顶依次执行
                                  ↓
                              所有指令执行完毕 → 广播结果
```

## 数据结构

### ICommand（原子指令）

```cpp
struct ICommand {
    virtual ~ICommand() = default;
    virtual void execute(FlychessGame& game) = 0;
    virtual std::string describe() const = 0;
};
```

每个卡牌效果最终都会分解为若干个 ICommand 原子指令。

### ResolutionStack（结算栈）

```
栈底 ←──────────────────── 栈顶
  cmd_1  cmd_2  cmd_3  ← push方向
                        → resolve方向（先执行 cmd_3）
```

- `push(cmd)`: 压入单个指令
- `pushEffect(effect)`: 逆序压入一系列指令（保证原序执行）
- `resolveAll(game)`: 从栈顶弹出一个执行一个，直到栈空
- 结算过程中若触发新卡牌，新指令压入栈顶，**后发先至**

### CardEffect（卡牌效果）

```cpp
struct CardEffect {
    int card_id;
    std::string card_name;
    int source_player;
    int target_player;
    std::vector<int> target_pieces;
    std::vector<std::unique_ptr<ICommand>> commands;
};
```

## 原子指令表

| op 名 | Command 类 | 参数 | 对应 game 方法 |
|-------|-----------|------|---------------|
| `set_dice` | SetDiceCommand | value (int) | `game.setDice(value)` 或 `game.setNextSix()` |
| `send_to_start` | SendToStartCommand | player, piece_id | `game.GetPlayer(p).SendChessPieceBackHome(id)` |
| `send_all_pre_to_start` | SendAllPreToStartCommand | player | `game.setAllPreBack(player)` |
| `move_piece` | MovePieceCommand | player, piece_id, steps | `game.MoveChessPiece(p, id, steps)` |
| `fly_piece` | FlyPieceCommand | player, piece_id | `game.FlyChessPiece(p, id)` |
| `broadcast` | BroadcastCommand | message (json) | （回调通知 server 广播） |

`broadcast` 比较特殊——它需要一个回调来通知 server 层。通过构造函数注入 `std::function<void(const nlohmann::json&)>` 解决。

## JSON DSL 格式

### 旧格式（当前）

```json
{"id": 0, "name": "6", "description": "使自己投出一个6", "function_time": 2, "image_path": "../assets/cards/6.jpg", "target_selection": 0}
```

### 新格式

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

### 变量替换

effect 参数中支持以下变量占位符，运行时替换为实际值：

| 变量 | 含义 | 来源 |
|------|------|------|
| `$caster` | 出牌玩家颜色 | use_card 消息的发送者 |
| `$target` | 目标玩家颜色 | use_card 消息的 target_id |
| `$selected_piece` | 选中的棋子 ID | use_card 消息的 piece_id |

### 复杂卡牌的逃生舱

```json
{
  "id": 10,
  "name": "时光倒流",
  "description": "撤销上一张卡牌的效果",
  "function_time": 0,
  "target_selection": 0,
  "effects": [
    {"op": "custom", "class": "TimeReverseCommand"}
  ]
}
```

`op: "custom"` 走 C++ 工厂创建自定义 ICommand 子类，可以写任意复杂逻辑。

## 连锁结算示例

场景：玩家 RED 打出"6"卡，玩家 BLUE 响应打出"极端天气"。

```
初始: ResolutionStack = []

1. RED 打出 "6" (set_dice 6)
   pushEffect: [SetDiceCommand(6)]
   ResolutionStack: [SetDice(6)]

2. 开始 resolveAll
   弹出 SetDice(6)，执行前触发 BLUE 响应

3. BLUE 打出 "极端天气" (send_all_pre_to_start $caster=BLUE)
   pushEffect: [SendAllPreToStartCommand(BLUE)]
   ResolutionStack: [SetDice(6), SendAllPreToStart(BLUE)]
                                     ↑ 栈顶

4. 继续 resolveAll
   弹出 SendAllPreToStart(BLUE) → 执行：BLUE 的待飞区棋子回起点
   弹出 SetDice(6) → 执行：RED 下次必出 6

5. 栈空，结算完毕 → 广播最终游戏状态
```

后发先至：BLUE 的极端天气在 RED 的 set_dice 之前生效。

## 服务端集成

`handleUseCard` 变为：

```
1. 从 card.json 查找卡牌定义
2. 解析 effects 数组 → 创建 ICommand 列表 → CardEffect
3. resolution_stack_.pushEffect(effect)
4. resolution_stack_.resolveAll(*game_)  ← 这里可能触发连锁
5. BroadCastPieceInfo() → 通知所有客户端
```

## 客户端需要配合的改动

客户端在收到卡牌效果结算完毕后，服务端会广播 `positions_update`（即现有的 `all_piece_info`）。客户端只需要响应这个消息重绘棋盘即可，不需要理解栈结算逻辑。

## 未来扩展

- **undo 支持**: ICommand 加 `undo()` 方法，实现"反制"类卡牌
- **条件指令**: `{"op": "if", "condition": "has_started_piece", "then": [...], "else": [...]}`
- **随机指令**: `{"op": "random", "options": [effect_a, effect_b]}` — 随机选择一个效果执行
- **动画事件**: ICommand 执行后 emit 事件给客户端做动画（棋子移动动画、粒子效果等）
