#include "../include/map.h"

namespace flychess_map{
    Map::Map() {
        // 
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

        // std::cout<<"HOME,size = " << grids.size() << std::endl;

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

        // std::cout<<"NORMAL,size = " << grids.size() << std::endl;

        // NORMAL,
        for (int i = 0; i < 5; i++){
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 4, grid_size + i * grid_size / 2, 53 + i, grid_size / 2, grid_size / 2, 0));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), map_size - grid_size - (i + 1) * grid_size / 2, grid_size * 4, 53 + i, grid_size / 2, grid_size / 2, 1));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size * 4, map_size - grid_size - (i + 1) * grid_size / 2, 53 + i, grid_size / 2, grid_size / 2, 2));
            grids.push_back(Grid(static_cast<int>(GridType::NORMAL), grid_size + i * grid_size / 2, grid_size * 4, 53 + i, grid_size / 2, grid_size / 2, 3));
        }

        // std::cout<<",size = " << grids.size() << std::endl;

        // TURN
        // Gridwidthheight0123
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

        // std::cout<<"TURN,size = " << grids.size() << std::endl;

        // GOAL
        int goal_x = map_size / 2;
        int goal_y = map_size / 2;
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 0, 0));
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 1, 1));
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 2, 2));
        grids.push_back(Grid(static_cast<int>(GridType::GOAL), goal_x, goal_y, 58, grid_size * 0.75, 3, 3));

        // std::cout<<"GOAL,size = " << grids.size() << std::endl;

        //START
        grids.push_back(Grid(static_cast<int>(GridType::START), grid_size * 2, 0, 0, grid_size, 0, static_cast<int>(game_utils::Color::UNDEFINED)));
        grids.push_back(Grid(static_cast<int>(GridType::START), map_size, grid_size * 2, 0, grid_size, 1, static_cast<int>(game_utils::Color::UNDEFINED)));
        grids.push_back(Grid(static_cast<int>(GridType::START), map_size - grid_size * 2, map_size, 0, grid_size, 2, static_cast<int>(game_utils::Color::UNDEFINED)));
        grids.push_back(Grid(static_cast<int>(GridType::START), 0, map_size - grid_size * 2, 0, grid_size, 3, static_cast<int>(game_utils::Color::UNDEFINED)));
        // std::cout<<"START,size = " << grids.size() << std::endl;
    
        // 0
        grids.push_back(Grid(static_cast<int>(GridType::NORMAL), 0, 0, -2, 0, 0, static_cast<int>(game_utils::Color::UNDEFINED)));
    }

    const GridInfo& Map::searchGridInfo(int position_id, int color, int chess_id) {
        // 
        if (position_id < -2 || position_id > 58) {
            std::cerr << "Position ID out of bounds: " << position_id << std::endl;
            throw std::out_of_range("Position ID out of bounds");
        }

        // 
        // std::cout<<"Searching for grid with position_id: " << position_id << ", color: " << color << ", chess_id: " << chess_id << std::endl;
        if (position_id == -1) {// HOME
            // std::cout<<"Searching for HOME grid"<<std::endl;
            int index = color + chess_id * 4;
            return grids[index].getGridInfo();
        }
        else if(position_id == -2) {
            // 
            // 0
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
            // md
            // md
            int start_index = 16;
            int end_index = 87;
            for (int i = start_index; i <= end_index; i++) {
                if (i > 52 && i < 58) {
                    // 
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
            // 
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
}

#ifdef __EMSCRIPTEN__

extern "C"{

    EMSCRIPTEN_KEEPALIVE
    const flychess_map::GridInfo* GetGridInfo(int index){
        if (index < 0 || index >= flychess_map::getGameMap().getGridsize()) {
            std::cerr << "Index out of bounds: " << index << std::endl;
            return nullptr; // 
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