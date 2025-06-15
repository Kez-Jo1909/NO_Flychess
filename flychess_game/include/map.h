#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <vector>
#include <emscripten/emscripten.h>
#include "./utils.h"

namespace flychess_map{

enum class GridType {
    NORMAL,
    START,
    HOME,     // 起始区
    GOAL,     // 终点区
    BRIDGE,   // 跳跃区
    TURN      // 转角点
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
    GridInfo grid_info; // 格子信息
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

private:
    const int grid_size = 80;  // 地图宽度
    const int map_size = 680;

    /**
     * @name grids
     * @brief 存储地图格子信息
     * @details 前四个元素为Home区，后4个元素为Goal区
     * @details 再后四个为START区
     */
    std::vector<Grid> grids; // 存储地图格子
};

static Map gameMap; // 全局地图对象

}

/*以下为JS接口*/

#ifdef __cplusplus
extern "C"{
#endif

// 返回地图格子信息
const flychess_map::GridInfo* GetGridInfo(int index);

int GetGridCount();

#ifdef __cplusplus
}
#endif


#endif