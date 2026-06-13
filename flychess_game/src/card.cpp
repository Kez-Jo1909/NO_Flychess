#include "../include/card.h"
#include <fstream>

namespace flychess_game {

    // ============================================================
    // 卡牌子类（客户端显示用，保留构造函数用于工厂创建）
    // ============================================================
    class Card_Six : public flychess_card {
    public:
        using flychess_card::flychess_card;
        Card_Six(const nlohmann::json &j) : flychess_card(j) { count_++; }
        ~Card_Six() { --count_; }
        static int getCount() { return count_; }
    private:
        static inline int count_ = 0;
    };

    class Card_ExtremeWeather : public flychess_card {
    public:
        using flychess_card::flychess_card;
        Card_ExtremeWeather(const nlohmann::json &j) : flychess_card(j) { count_++; }
        ~Card_ExtremeWeather() { --count_; }
        static int getCount() { return count_; }
    private:
        static inline int count_ = 0;
    };

    // ============================================================
    // 卡牌工厂注册（仅用于客户端创建卡牌实例显示）
    // ============================================================
    void register_cards() {
        auto& factory = CardFactory::get_instance();

        factory.register_card(0, [](const nlohmann::json& j) {
            return std::make_unique<Card_Six>(j);
        });
        factory.register_card(1, [](const nlohmann::json& j) {
            return std::make_unique<Card_ExtremeWeather>(j);
        });
    }

    // ============================================================
    // 卡牌配置加载
    // ============================================================
    nlohmann::json load_card_config(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[CardConfig] 无法打开文件: " << filepath << std::endl;
            return nlohmann::json::array();
        }
        nlohmann::json card_list;
        try {
            file >> card_list;
        } catch (const std::exception& e) {
            std::cerr << "[CardConfig] JSON 解析失败: " << e.what() << std::endl;
            return nlohmann::json::array();
        }
        std::cout << "[CardConfig] 加载了 " << card_list.size() << " 张卡牌配置" << std::endl;
        return card_list;
    }

    nlohmann::json find_card_by_id(const nlohmann::json& card_list, int card_id) {
        for (const auto& card : card_list) {
            if (card.at("id").get<int>() == card_id) {
                return card;
            }
        }
        std::cerr << "[CardConfig] 未找到卡牌 id=" << card_id << std::endl;
        return nlohmann::json();
    }

}  // namespace flychess_game
