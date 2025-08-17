#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include "./utils.h"

namespace flychess_map{

enum class GridType {
    NORMAL,
    START,
    HOME,     // 
    GOAL,     // 
    BRIDGE,   // 
    TURN      // 
};

struct GridInfo {
    int type;
    int position_x;
    int position_y;
    int id;
    int width;
    int height;
    int color;

    GridInfo(int type, int x, int y, int id, int width, int height, int color)
        : type(type), position_x(x), position_y(y), id(id), width(width), height(height), color(color) {}
};

class Grid{
public:
    Grid(int type, int x, int y, int id, int width, int height, int color)
        : grid_info(type, x, y, id, width, height, color) {};

    inline const GridInfo& getGridInfo() const {
        return grid_info;
    }
private:
    GridInfo grid_info; // 
};


class Map{
public:
    Map();

    inline int getGridsize() const {
        return grids.size();
    }

    inline const std::vector<Grid>& getGrids() const {
        return grids;
    }

    inline const Grid& getGrid(int index) const {
        return grids[index];
    }

    const GridInfo& searchGridInfo(int position_id, int color, int chess_id);

private:
    const int grid_size = 80;
    const int map_size = 680;

    /**
     * @name grids
     * @brief 
     * @details Home4Goal
     * @details START
     */
    std::vector<Grid> grids; // 
};

inline Map& getGameMap() {
    static Map instance;
    return instance;
}

int GetGridCount();
const GridInfo* GetGridInfo(int index);

}

/*JS*/
#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C"{
#endif

// 
const flychess_map::GridInfo* GetGridInfo(int index);

int GetGridCount();

#ifdef __cplusplus
}
#endif
#endif // __EMSCRIPTEN__

#endif