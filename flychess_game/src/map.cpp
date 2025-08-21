#include "../include/map.h"

namespace flychess_map{
    Map::Map() {
        this->LoadMapFromJson();
        std::cout<< "Map loaded from JSON, total grids: " << grids.size() << std::endl;
    }

    const GridInfo& Map::searchGridInfo(int position_id, int color, int chess_id) {
        // 检查越界
        if (position_id < -2 || position_id > 58) {
            std::cerr << "Position ID out of bounds: " << position_id << std::endl;
            throw std::out_of_range("Position ID out of bounds");
        }

        // 开始匹配棋子位置
        // std::cout<<"Searching for grid with position_id: " << position_id << ", color: " << color << ", chess_id: " << chess_id << std::endl;
        if (position_id == -1) {// HOME
            // std::cout<<"Searching for HOME grid"<<std::endl;
            int index = color + chess_id * 4;
            return grids[index].getGridInfo();
        }
        else if(position_id == -2) {
            // 已经完成的棋子
            // 妈的不管了返回一个长宽为0的得了
            return grids[96].getGridInfo();
        }
        else if (position_id == 58) {// GOAL
            // std::cout<<"Searching for GOAL grid"<<std::endl;
            int index = color + 88;
            return grids[index].getGridInfo();
        }
        else if (position_id == 0) {// START
            // std::cout<<"Searching for START grid"<<std::endl;
            int index = 92 + color;
            return grids[index].getGridInfo();
        }
        else if (position_id >= 1 && position_id <= 52) {
            // NORMAL
            // std::cout<<"Searching for NORMAL grid"<<std::endl;
            // md不管了直接遍历
            // md只能写常规遍历
            int start_index = 16;
            int end_index = 87;
            for (int i = start_index; i <= end_index; i++) {
                if (i > 52 && i < 58) {
                    // 跳过终点前路径
                    continue;
                }

                if (grids[i].getGridInfo().id == position_id) {
                    // std::cout<<"Found grid at index: " << i << std::endl;
                    return grids[i].getGridInfo();
                }
            }
            std::cout<<"Searching failed in normal situation"<<std::endl;
            throw std::runtime_error("Search failed: non-exist grid");
        }
        else if(position_id > 52 && position_id < 58){
            // 终点前路径
            // std::cout<<"Searching for pre-goal path grid"<<std::endl;
            // std::cout<<"color: " << color << std::endl;
            int index = 52 + color + (position_id - 53) * 4;
            return grids[index].getGridInfo();
        }
        else{
            std::cout<<"Searching failed"<<std::endl;
            throw std::runtime_error("Search failed: Invalid position_id or chess_id");
        }
    }

    int GetGridCount() {
        return flychess_map::getGameMap().getGridsize();
    }

    const GridInfo* GetGridInfo(int index) {
        if (index < 0 || index >= flychess_map::getGameMap().getGridsize()) {
            std::cerr << "Index out of bounds: " << index << std::endl;
            return nullptr; // 返回空指针表示索引越界
        }
        return &flychess_map::getGameMap().getGrid(index).getGridInfo();
    }  

    void Map::SaveToJson() {
        nlohmann::json j;
        for (auto grid : grids) {
            auto grid_info = grid.getGridInfo();
            j["grids"].push_back({
                {"type", grid_info.type},
                {"position_x", grid_info.position_x},
                {"position_y", grid_info.position_y},
                {"id", grid_info.id},
                {"width", grid_info.width},
                {"height", grid_info.height},
                {"color", grid_info.color}
            });
        }
        std::ofstream file("game_map.json");
        file << j.dump(4);
        std::cout << "Game map saved to game_map.json" << std::endl;
    }

    void Map::LoadMapFromJson() {
        std::string filename = "../config/game_map.json";
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开JSON文件: " + filename);
        }

        nlohmann::json j;
        file >> j;
        grids.clear();
        for (const auto& g : j["grids"]) {
            grids.emplace_back(
                g["type"].get<int>(),
                g["position_x"].get<int>(),
                g["position_y"].get<int>(),
                g["id"].get<int>(),
                g["width"].get<int>(),
                g["height"].get<int>(),
                g["color"].get<int>()
            );
        }
    }
}

#ifdef __EMSCRIPTEN__

extern "C"{

    EMSCRIPTEN_KEEPALIVE
    const flychess_map::GridInfo* GetGridInfo(int index){
        if (index < 0 || index >= flychess_map::getGameMap().getGridsize()) {
            std::cerr << "Index out of bounds: " << index << std::endl;
            return nullptr; // 返回空指针表示索引越界
        }
        return &flychess_map::getGameMap().getGrid(index).getGridInfo();
    }

    EMSCRIPTEN_KEEPALIVE
    int GetGridCount() {
        // std::cout<<flychess_map::getGameMap().getGridsize()<<std::endl;
        return flychess_map::getGameMap().getGridsize();
    }
}
#endif