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