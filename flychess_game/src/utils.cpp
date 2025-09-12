#include "../include/utils.h"

namespace game_utils {

    CardInfo getCardById(const std::string& filepath, int target_id) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件: " + filepath);
        }

        nlohmann::json j;
        file >> j;
        file.close();

        for (const auto& item : j) {
            if (item.at("id").get<int>() == target_id) {
                return CardInfo{
                    item.at("id").get<int>(),
                    item.at("name").get<std::string>(),
                    item.at("description").get<std::string>(),
                    static_cast<game_utils::CardFunctionTime>(item.at("function_time").get<int>()),
                    item.at("image_path").get<std::string>(),
                    item.at("target_selection").get<int>()
                };
            }
        }

        throw std::runtime_error("未找到对应 id 的卡牌: " + std::to_string(target_id));
    }
    

}// namespace game_utils

