#include "../include/map.h"

namespace flychess_map{
    Map::Map() {
        // 初始化地图
        grids.clear();
        // HOME
        grids.push_back(Grid(static_cast<int>(GridType::HOME), 0, 0, -1, grid_size * 2, grid_size * 2, static_cast<int>(game_utils::Color::RED)));
        grids.push_back(Grid(static_cast<int>(GridType::HOME), map_size - grid_size * 2, 0, -1, grid_size * 2, grid_size * 2, static_cast<int>(game_utils::Color::BLUE)));
        grids.push_back(Grid(static_cast<int>(GridType::HOME), map_size - grid_size * 2, map_size - grid_size * 2, -1, grid_size * 2, grid_size * 2, static_cast<int>(game_utils::Color::GREEN)));
        grids.push_back(Grid(static_cast<int>(GridType::HOME), 0, map_size - grid_size * 2, -1, grid_size * 2, grid_size * 2, static_cast<int>(game_utils::Color::YELLOW)));

        // NORMAL
        for (int i = 0; i < 5; i++){
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), 3 * grid_size + i * grid_size / 2, 0, i + 2, grid_size / 2, grid_size, (i + 2) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size - grid_size * 3 - (i + 1) * grid_size / 2, map_size - grid_size, 28 + i, grid_size / 2, grid_size, i % 4));
        
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), 0, map_size - grid_size * 3  - (i + 1) * grid_size / 2, 41 + i, grid_size, grid_size / 2, (i + 1) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size - grid_size, 3 * grid_size + i * grid_size / 2, 15 + i, grid_size, grid_size / 2, (i + 3) % 4));
        }

        for (int i = 0; i < 2; i++){
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 5.5, grid_size + i * grid_size / 2, 8 + i, grid_size, grid_size / 2, i % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 5.5, map_size - grid_size * 2 + i * grid_size / 2, 25 + i, grid_size, grid_size / 2, (i + 1) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 2, grid_size * 1.5 - i * grid_size / 2, 51 + i, grid_size, grid_size / 2, (i + 3) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 2, map_size - grid_size * 1.5 - i * grid_size / 2, 34 + i, grid_size, grid_size / 2, (i + 2) % 4));
            
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 6.5 + i * grid_size / 2, grid_size * 2, 12 + i, grid_size / 2, grid_size, i % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size -  grid_size - (i + 1) * grid_size / 2, grid_size * 5.5, 21 + i, grid_size / 2, grid_size, (i + 1) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 2 - (i + 1) * grid_size / 2, map_size - grid_size * 3, 38 + i, grid_size / 2, grid_size, (i + 2) % 4));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size + i * grid_size / 2, grid_size * 2, 47 + i, grid_size / 2, grid_size, (i + 3) % 4));
        }

        for (int i = 0; i < 5; i++){
            grids.push_back(Grid(static_cast<int>(GridType::GOAL), grid_size * 4, grid_size + i * grid_size / 2, 53 + i, grid_size / 2, grid_size / 2, 0));
            grids.push_back(Grid(static_cast<int>(GridType::GOAL), map_size - grid_size - (i + 1) * grid_size / 2, grid_size * 4, 53 + i, grid_size / 2, grid_size / 2, 1));
            grids.push_back(Grid(static_cast<int>(GridType::GOAL), grid_size * 4, map_size - grid_size - (i + 1) * grid_size / 2, 53 + i, grid_size / 2, grid_size / 2, 2));
            grids.push_back(Grid(static_cast<int>(GridType::GOAL), grid_size + i * grid_size / 2, grid_size * 4, 53 + i, grid_size / 2, grid_size / 2, 3));
        }
    }
}

extern "C"{

    EMSCRIPTEN_KEEPALIVE
    const flychess_map::Grid* GetGridInfo(int index){
        if (index < 0 || index >= flychess_map::gameMap.getGridsize()) {
            std::cerr << "Index out of bounds: " << index << std::endl;
            return nullptr; // 返回空指针表示索引越界
        }
        return &flychess_map::gameMap.getGrid(index);
    }

    EMSCRIPTEN_KEEPALIVE
    int GetGridCount() {
        std::cout<<flychess_map::gameMap.getGridsize()<<std::endl;
        return flychess_map::gameMap.getGridsize();
    }
        

}