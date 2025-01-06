#include <config.h>

//  标准库
#include <exception>
#include <filesystem>
#include <fstream>

//  MiraiCP
#include <MiraiCP.hpp>
#include <json.hpp>

namespace config {

const fs::path current_path = fs::current_path() / "data" / "tech.eritquearcus.miraicp" / "Poly";

namespace fs = std::filesystem;
using namespace MiraiCP;

//  class data definition begin
data::data(const nlohmann::json& config_json)
    : server_map(config_json.at("server")), poly_path{current_path / config_json.at("poly_config")} {}
//  class data definition end

const data& get() {
    static data config_data = config::read();
    return config_data;
}

nlohmann::json read() {
    //  假设当前目录下有配置文件
    //  拼接配置文件路径, 预期与动态库处于同一目录下
    auto config_path = current_path / "config.json";
    Logger::logger.info("配置文件路径: ", current_path.string());

    //  如果配置文件不存在, 那么我们无法获悉
    //  向哪些服务器查询数据, 因此这里选择直接终止程序
    if (!fs::exists(config_path)) {
        Logger::logger.error("插件需要配置文件运行, 但配置文件不存在! 查找路径: ", config_path.string());
        std::terminate();
    }

    //  返回配置文件
    return nlohmann::json::parse( std::ifstream(config_path) );
}

}   //  end of namespace config