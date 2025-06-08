#include "../include/game.h"

namespace flychess_game {
    
    FlyChessGame::FlyChessGame() {
        // 默认构造函数实现
        std::cout << "FlyChessGame created with default constructor." << std::endl;
    }

    FlyChessGame::FlyChessGame(int playerCount, int machineCount) {// warning: 这里不对传入的玩家数做检查
        // 带参数的构造函数实现
        std::cout << "FlyChessGame created with playerCount: " << playerCount 
                  << " and machineCount: " << machineCount << std::endl;
        
        // 初始化玩家列表
        for (int i = 0; i < playerCount; i++) {
            players.push_back(new Player());
        }
        
        // 如果有机器玩家，则添加机器玩家
        for (int i = 0; i < machineCount; i++) {
            players.push_back(new Player()); // 这里可以根据需要设置机器玩家的属性
        }

        for(int i = 0; i < players.size(); i++){
            std::cout << "Player " << i << " initialized." << std::endl;
            // 这里可以对每个玩家进行初始化设置
            players[i]->SetColor(i % 4); // 设置玩家颜色，假设最多4种颜色
        }

        // TODO : 是不是得更好的处理异常状态
        if(players.size() > 4){
            std::cerr << "Warning: Player count exceeds 4, which is not supported in FlyChessGame." << std::endl;
        }
        else if(players.empty()){
            std::cerr << "Warning: No players initialized in FlyChessGame." << std::endl;
        }
    }

    void GameProcess(){
        std::cout << "GameProcess started." << std::endl;
        
        // 创建游戏实例
        // TODO : 这里可以根据需要传入玩家数量和机器玩家数量
        FlyChessGame game(1, 0); // 假设有1个玩家和0个机器玩家(测试用)
        std::cout << "GameProcess initialized with " << game.GetPlayerCount() << " players." << std::endl;

        // 游戏主循环逻辑
        while (!game.IsGameOver()) {
            // TODO : 这里可以添加游戏逻辑
            std::cout << "Game is running..." << std::endl;
            // 模拟游戏进行
            game.SetGameOver(); // 这里为了测试，直接结束游戏
        }

        std::cout << "GameProcess ended." << std::endl;

        // 清理资源
        game.DeleteGame(); // 调用删除游戏资源的函数

        // TODO : 这里可以添加游戏结束后的处理逻辑
        std::cout << "GameProcess completed." << std::endl;

        // 删除游戏实例
        // warning：如果使用了new创建的对象，需要手动delete
        // delete &game; // 这里不需要，因为game是栈对象，不需要手动删除
    }
    
}


extern "C"{

    EMSCRIPTEN_KEEPALIVE
    int GetRandom() {
        int min = 1;
        int max = 6;
        int random_number = game_utils::get_random(min, max);
        std::cout << "Random number between " << min << " and " << max << ": " << random_number << std::endl;
        return random_number;
    }

    EMSCRIPTEN_KEEPALIVE
    void GameProcessLink(){
        flychess_game::GameProcess();
    }

    EMSCRIPTEN_KEEPALIVE
    void FrontendTest(){

    }

}