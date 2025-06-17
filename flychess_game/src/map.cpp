#include "../include/map.h"

namespace flychess_map{
    Map::Map() {
        // 初始化地图
        grids.clear();

        // HOME
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++){
                grids.push_back(Grid(static_cast<int>(GridType::HOME), i * grid_size, j * grid_size, -1, grid_size, grid_size, static_cast<int>(game_utils::Color::RED)));
                grids.push_back(Grid(static_cast<int>(GridType::HOME), map_size - grid_size * 2 + i * grid_size, j * grid_size, -1, grid_size, grid_size, static_cast<int>(game_utils::Color::BLUE)));
                grids.push_back(Grid(static_cast<int>(GridType::HOME), map_size - grid_size * 2 + i * grid_size, map_size - grid_size * 2 + j * grid_size, -1, grid_size, grid_size, static_cast<int>(game_utils::Color::GREEN)));
                grids.push_back(Grid(static_cast<int>(GridType::HOME), i * grid_size, map_size - grid_size * 2 + j * grid_size, -1, grid_size, grid_size, static_cast<int>(game_utils::Color::YELLOW)));
            }
        }

        // NORMAL
        for (int i = 0; i < 5; i++){
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), 3 * grid_size + i * grid_size / 2, 0, 52 - i, grid_size / 2, grid_size, (i + 2) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size - grid_size * 3 - (i + 1) * grid_size / 2, map_size - grid_size, 26 - i, grid_size / 2, grid_size, i % 4));
        
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), 0, map_size - grid_size * 3  - (i + 1) * grid_size / 2, 13 - i, grid_size, grid_size / 2, (i + 1) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size - grid_size, 3 * grid_size + i * grid_size / 2, 39 - i, grid_size, grid_size / 2, (i + 3) % 4));
        }

        for (int i = 0; i < 2; i++){
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 5.5, grid_size + i * grid_size / 2, 46 - i, grid_size, grid_size / 2, i % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 5.5, map_size - grid_size * 2 + i * grid_size / 2, 29 - i, grid_size, grid_size / 2, (i + 1) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 2, grid_size * 1.5 - i * grid_size / 2, 3 - i, grid_size, grid_size / 2, (i + 3) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 2, map_size - grid_size * 1.5 - i * grid_size / 2, 20 - i, grid_size, grid_size / 2, (i + 2) % 4));
            
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 6.5 + i * grid_size / 2, grid_size * 2, 42 - i, grid_size / 2, grid_size, i % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size -  grid_size - (i + 1) * grid_size / 2, grid_size * 5.5, 33 - i, grid_size / 2, grid_size, (i + 1) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 2 - (i + 1) * grid_size / 2, map_size - grid_size * 3, 16 - i, grid_size / 2, grid_size, (i + 2) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size + i * grid_size / 2, grid_size * 2, 7 - i, grid_size / 2, grid_size, (i + 3) % 4));
        }

        // NORMAL,终点前路径
        for (int i = 0; i < 5; i++){
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 4, grid_size + i * grid_size / 2, 53 + i, grid_size / 2, grid_size / 2, 0));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size - grid_size - (i + 1) * grid_size / 2, grid_size * 4, 53 + i, grid_size / 2, grid_size / 2, 1));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 4, map_size - grid_size - (i + 1) * grid_size / 2, 53 + i, grid_size / 2, grid_size / 2, 2));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size + i * grid_size / 2, grid_size * 4, 53 + i, grid_size / 2, grid_size / 2, 3));
        }

        // TURN
        // 也用Grid结构体表示，发送直角三角形直角点坐标，width发送直角边长，height发送类型，即正方形左上角0，右上角1，右下角2，左下角3
        grids.push_back(Grid(static_cast<int>(GridType::TURN), grid_size * 3, grid_size, 1, grid_size, 2, 1));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), grid_size * 5.5, grid_size, 47, grid_size, 3, 3));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 5.5, grid_size * 2, 44, grid_size, 0, 2));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 6.5, grid_size * 3, 43, grid_size, 2, 3));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), map_size - grid_size, grid_size * 3, 40, grid_size, 3, 2));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), map_size - grid_size, map_size - grid_size * 3, 34, grid_size, 0, 0));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 6.5, map_size - grid_size * 3, 31, grid_size, 1, 3));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 5.5, map_size - grid_size * 2, 30, grid_size, 3, 0));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), grid_size * 5.5, map_size - grid_size, 27, grid_size, 0, 3));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), grid_size * 3, map_size - grid_size, 21, grid_size, 1, 1));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 3, map_size - grid_size * 2, 18, grid_size, 2, 0));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 2, map_size - grid_size * 3, 17, grid_size, 0, 1));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), grid_size, map_size - grid_size * 3, 14, grid_size, 1, 0));
        grids.push_back(Grid(static_cast<int>(GridType::TURN), grid_size, grid_size * 3, 8, grid_size, 2, 2));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 2, grid_size * 3, 5, grid_size, 3, 1));
        grids.push_back(Grid(static_cast<int>(GridType::BRIDGE), grid_size * 3, grid_size * 2, 4, grid_size, 1, 2));

        // GOAL
        int goal_x = map_size / 2;
        int goal_y = map_size / 2;
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 0, 0));
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 1, 1));
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 2, 2));
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 3, 3));
    }
}

extern "C"{

    EMSCRIPTEN_KEEPALIVE
    const flychess_map::GridInfo* GetGridInfo(int index){
        if (index < 0 || index >= flychess_map::gameMap.getGridsize()) {
            std::cerr << "Index out of bounds: " << index << std::endl;
            return nullptr; // 返回空指针表示索引越界
        }
        return &flychess_map::gameMap.getGrid(index).getGridInfo();
    }

    EMSCRIPTEN_KEEPALIVE
    int GetGridCount() {
        // std::cout<<flychess_map::gameMap.getGridsize()<<std::endl;
        return flychess_map::gameMap.getGridsize();
    }
        

}