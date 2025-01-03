#include "detailed.h"
#include <commands/poly.h>

#include <fstream>

using json = nlohmann::json;

poly::poly(std::filesystem::path poly_path) {
    std::ifstream poly_file_stream{ poly_path };

    if (!poly_file_stream) [[unlikely]] {
        Logger::logger.error("无法打开配置文件!");
        return;
    }

    auto poly_json = json::parse( poly_file_stream );
    auto phrase_json = poly_json.at("phrase");

    if (!phrase_json.empty()) {
        std::vector<std::string> result{};
        result.reserve(phrase_json.size());

        for (auto iter = phrase_json.rbegin(); iter != phrase_json.rend(); iter++) {
            result.push_back(iter.key());
        }

        m_message_vector = std::move(result);
        Logger::logger.info("已读取 Poly 消息数: {}", m_message_vector.size());
    } else {
        Logger::logger.warning("Poly 的消息文件内未能读取到任何消息!");
    }   
}

const std::vector<std::string> poly::get_message() {
    const auto& config_data = config::get();
    static poly static_poly = config_data.poly_path;
    return static_poly.m_message_vector;
}

void command::send_poly_message(GroupMessageEvent event) {
    static auto poly_message = poly::get_message();
    static detailed::rng generator{0, poly_message.size()};

    auto message = poly_message.at(generator.yield());

    event.group.sendMessage(message);
}